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
import xdecoder
import argparse
import json

FLAGS = None

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("Hello, world")

class Application(tornado.web.Application):
    def __init__(self):
        settings = {
            "static_path": os.path.join(os.path.dirname(__file__), "static"),
        }
        handlers = [
            (r'/', MainHandler),
        ]

        tornado.web.Application.__init__(self, handlers, **settings)
        logging.basicConfig(level = logging.DEBUG, 
                            format = '%(levelname)s %(asctime)s (%(filename)s:%(funcName)s():%(lineno)d) %(message)s') 
        self.manager = xdecoder.ResourceManager()
        with open(FLAGS.config_file, "r") as fid:
            self.config = json.load(fid)
        self.set_parameters()
        self.manager.init()
    
    def set_parameters(self):
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

    def start(self):
        logging.info('server start, port: %d...' % 8888)
        self.listen(8888)
        tornado.ioloop.IOLoop.current().start()

if __name__ == "__main__":
    usage = 'usage: python main.py main.ini'
    parser = argparse.ArgumentParser(description=usage)
    parser.add_argument('config_file', help='json config file')
    FLAGS = parser.parse_args()
    Application().start()

