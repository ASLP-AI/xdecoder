# Created on 2018-07-02
# Author: Binbin Zhang

import argparse
import wave
import websocket
import struct
import json
import platform

try:
    import thread
except ImportError:
    import _thread as thread
import time

FLAGS = None

def on_message(ws, message):
    msg = json.loads(message)
    print(msg)

def on_error(ws, error):
    print(error)
    pass

def on_close(ws):
    print("### closed ###")

def on_open(ws):
    def run(*args):
        wav_reader = wave.open(FLAGS.wave_file, 'rb')
        chunk = int(16000 * 0.5) # 0.5s as a chunk
        done = False
        while not done:
            package = []
            while len(package) < chunk and not done:
                raw_data = wav_reader.readframes(1)
                if len(raw_data) == 0:
                    done = True
                    break
                else:  
                    data = struct.unpack('<h', raw_data)
                    package.append(data[0])
            if len(package) > 0:
                message = struct.pack('!%uh' % len(package), *package)
                ws.send(message, websocket.ABNF.OPCODE_BINARY)
                print('Send', len(message))
                time.sleep(0.5)
        ws.send('<EOS>')
        ws.close()

    thread.start_new_thread(run, ())

if __name__ == '__main__':
    usage = 'Simulate a websocket client request'
    parser = argparse.ArgumentParser(description=usage)
    parser.add_argument('wave_file', help='wav file for test')
    FLAGS = parser.parse_args()
    
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://localhost:10086/ws/decode",
                                header=['Client-Info: %s' % platform.version() ],
                                on_open = on_open,
                                on_message = on_message,
                                on_error = on_error,
                                on_close = on_close)

    ws.run_forever()

