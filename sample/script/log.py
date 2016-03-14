__author__ = 'wolfhead'

import logging
import logging.handlers

fmt = '[%(levelname)s] (%(threadName)-10s) %(message)s'
formatter = logging.Formatter(fmt)

handler = logging.handlers.RotatingFileHandler("benchmark.log", maxBytes=10 * 1024 * 1024, backupCount=50)
handler.setFormatter(formatter)
handler.setLevel(logging.DEBUG)

logger = logging.getLogger('benchmark')
logger.addHandler(handler)
logger.setLevel(logging.DEBUG)