# coding=utf-8
# Created on 2018-07-01
# Author: Binbin Zhang

import os
import sys
import threading
import time
import wave
import logging
import struct
import socket
import tornado.ioloop
import tornado.web
import tornado.websocket
import hashlib
import xdecoder
import argparse
import json
import dbhelper

FLAGS = None

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        try:
            self.user_agent = self.request.headers['User-Agent']
        except KeyError as name_err:
            logging.warning("DecoderWebsocketHandler: No 'User-Agent' key")
            self.user_agent = 'Unknown'
        print(self.user_agent)
        self.write("Hello, world %s" % socket.gethostname())

class HistoryHandler(tornado.web.RequestHandler):
    def get(self):
        page = int(self.get_argument('page', 0, strip=True))
        page_items = int(self.get_argument('page_items', 10, strip=True))
        db = dbhelper.DbHelper(**self.application.db_config)
        num_pages = int((db.get_history_count() - 1) / page_items + 1);
        if page < 0: page = 0
        if page >= num_pages: page = num_pages - 1
        offset = page * page_items
        start_page = page - 5
        if start_page < 0: start_page = 0
        end_page = page + 5
        if end_page > num_pages: end_page = num_pages - 1

        history = db.get_all_history(page * page_items, page_items)
        self.render("static/history.html", history = history,
                                           page_items = page_items,
                                           page = page,
                                           num_pages = num_pages,
                                           start_page = start_page,
                                           end_page = end_page)

class EchoWebSocketHandler(tornado.websocket.WebSocketHandler):
    def open(self):
        print("WebSocket opened")

    def on_message(self, message):
        self.write_message(u"You said: " + message)

    def on_close(self):
        print("WebSocket closed")

class DecodeWebSocketHandler(tornado.websocket.WebSocketHandler):
    def check_origin(self, orgin):
        return True

    def open(self):
        self.recognizer = xdecoder.Recognizer()
        self.application.manager.add_recognizer(self.recognizer)
        try:
            self.client_info = self.request.headers['Client-Info']
        except KeyError as name_err:
            logging.warning("DecoderWebsocketHandler: No 'Client-Info' key")
            self.client_info = 'Unknown'
        self.wav_data = b''
        self.results = []

    def on_message(self, message):
        if message == '<EOS>':
            self.recognizer.set_done()
        else:
            assert(len(message) % 2 == 0)
            size = int(len(message) / 2)
            short_data = struct.unpack('!%dh' % size, message)
            wav_data = struct.pack('<%dh' % size, *short_data)
            self.wav_data += wav_data
            float_data = xdecoder.FloatVector(size)
            for i in range(len(short_data)):
                float_data[i] = float(short_data[i])
            self.recognizer.add_wav(float_data)

        text_result = self.recognizer.get_result()
        arr = text_result.split(':')
        status = arr[0].strip()
        result = ''
        if len(arr) == 2: result = arr[1].strip()
        json_result = json.dumps({ "status": status,
                                   "result": result })
        if status == 'final' and result != '':
            self.results.append(result)
        self.write_message(json_result)

    def on_close(self):
        time_str = str(int(time.time()))
        m = hashlib.md5()
        m.update(time_str.encode('utf8'))
        wav_name = time_str + '_' +  m.hexdigest()[:10] + '.wav'
        wav_path = self.application.wav_dir + '/' + wav_name
        self.write_wav_file(wav_path, self.wav_data)
        all_result = '\n'.join(self.results)
        if self.application.use_db:
            db = dbhelper.DbHelper(**self.application.db_config)
            db.insert(wav_path, all_result, self.client_info)

    def write_wav_file(self, file_name, data):
        fid = wave.open(file_name, 'wb')
        fid.setnchannels(1)
        fid.setsampwidth(2)
        fid.setframerate(16000)
        fid.writeframes(data)
        fid.close()
        logging.info('write new wav file %s' % file_name)

class Application(tornado.web.Application):
    def __init__(self):
        settings = {
            "static_path": os.path.join(os.path.dirname(__file__), "static"),
        }
        handlers = [
            (r'/', MainHandler),
            (r'/history', HistoryHandler),
            (r'/ws/echo', EchoWebSocketHandler),
            (r'/ws/decode', DecodeWebSocketHandler),
        ]

        tornado.web.Application.__init__(self, handlers, **settings)
        logging.basicConfig(level = logging.DEBUG,
                            format = '%(levelname)s %(asctime)s (%(filename)s:%(funcName)s():%(lineno)d) %(message)s')
        with open(FLAGS.config_file, "r") as fid:
            self.config = json.load(fid)

        self.init_wav_dir()
        logging.info("init wav dir ok")
        time.sleep(10)
        self.init_db()
        logging.info("init database ok")
        self.init_manager()
        logging.info("init recognition engine ok")

    def init_wav_dir(self):
        self.wav_dir = self.config["wav_dir"]
        if not os.path.exists(self.wav_dir):
            os.makedirs(self.wav_dir)

    def init_db(self):
        self.use_db = self.config["use_db"]
        if self.use_db:
            self.db_config = self.config["db"]
            db = dbhelper.DbHelper(**self.db_config)
            if not db.asr_table_exist():
                db.create_asr_table()

    def init_manager(self):
        self.manager = xdecoder.ResourceManager()
        self.manager.set_thread_pool_size(self.config["runtime"]["thread_pool_size"])

        self.manager.set_beam(self.config["decoder"]["beam"])
        self.manager.set_max_active(self.config["decoder"]["max_active"])
        self.manager.set_acoustic_scale(self.config["decoder"]["acoustic_scale"])
        self.manager.set_skip(self.config["decoder"]["skip"])
        self.manager.set_max_batch_size(self.config["decoder"]["max_batch_size"])
        self.manager.set_hclg(self.config["decoder"]["hclg"])
        self.manager.set_tree(self.config["decoder"]["tree"])
        self.manager.set_pdf_prior(self.config["decoder"]["pdf_prior"])
        self.manager.set_lexicon(self.config["decoder"]["lexicon"])

        self.manager.set_am_net(self.config["am"]["net"])
        self.manager.set_am_num_bins(self.config["am"]["num_bins"])
        self.manager.set_am_left_context(self.config["am"]["left_context"])
        self.manager.set_am_right_context(self.config["am"]["right_context"])
        self.manager.set_am_cmvn(self.config["am"]["cmvn"])

        self.manager.set_vad_net(self.config["vad"]["net"])
        self.manager.set_vad_num_bins(self.config["vad"]["num_bins"])
        self.manager.set_vad_left_context(self.config["vad"]["left_context"])
        self.manager.set_vad_right_context(self.config["vad"]["right_context"])
        self.manager.set_vad_cmvn(self.config["vad"]["cmvn"])
        self.manager.set_silence_thresh(self.config["vad"]["silence_thresh"]);
        self.manager.set_silence_to_speech_thresh(self.config["vad"]["silence_to_speech_thresh"])
        self.manager.set_speech_to_sil_thresh(self.config["vad"]["speech_to_silence_thresh"])
        self.manager.set_endpoint_trigger_thresh(self.config["vad"]["endpoint_trigger_thresh"])
        self.manager.init()

    def start(self):
        logging.info('server start, port: %d...' % self.config["runtime"]["port"])
        self.listen(self.config["runtime"]["port"])
        tornado.ioloop.IOLoop.current().start()

if __name__ == "__main__":
    usage = 'usage: python main.py config.json'
    parser = argparse.ArgumentParser(description=usage)
    parser.add_argument('config_file', help='json config file')
    FLAGS = parser.parse_args()
    Application().start()

