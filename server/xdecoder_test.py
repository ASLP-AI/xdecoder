import xdecoder

manager = xdecoder.ResourceManager()
manager.set_beam(13.0)
manager.set_max_active(7000)
manager.set_net('exp/am.net')
manager.init()
