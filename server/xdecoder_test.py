import wave
import xdecoder
import struct
import time

manager = xdecoder.ResourceManager()
manager.set_beam(13.0)
manager.set_max_active(7000)
manager.set_acoustic_scale(0.1)
manager.set_skip(0)
manager.set_max_batch_size(64)
manager.set_num_bins(40)
manager.set_left_context(5)
manager.set_right_context(5)

manager.set_cmvn('exp/cmvn')
manager.set_hclg('exp/hclg')
manager.set_tree('exp/tree')
manager.set_net('exp/am.net')
manager.set_pdf_prior('exp/pdf_prior')
manager.set_lexicon('exp/words.txt')
manager.init()

recognizer = xdecoder.Recognizer()
manager.add_recognizer(recognizer)

wav_reader = wave.open('res/test.wav', 'rb')
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
        pcm_data = xdecoder.FloatVector(len(package))
        for i in range(len(package)):
            pcm_data[i] = float(package[i])
        recognizer.add_wav(pcm_data)
        print('RESULT:', recognizer.get_result())
        time.sleep(0.5)

wav_reader.close()
recognizer.set_done()
print('RESULT:', recognizer.get_result())

