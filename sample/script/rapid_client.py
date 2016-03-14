import socket
import struct 

class RapidClient(object):

  def connect(self, host, port):
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    self.sock.connect((host, port))

  def send_recv(self, data):

    size = 16 + len(data)
    seqid = 0
    message_type = 0
    message_version = 0
    message_cmdid = 0;
    message_extra = 0;

    msg = struct.pack("IIBBHI", size, seqid, message_type,\
      message_version, message_cmdid, message_extra)
    msg += data

    self.sock.send(msg);

    res = self.sock.recv(16);
    header = struct.unpack("IIBBHI", res)

    if header[0] < 16 :
      raise ValueError("package header length error:%d" % header[0])
    if 0 != header[1] :
      raise ValueError("package seq not match:%d" % header[1])
    if 0 != header[3] :
      raise ValueError("package version not match:%d" % header[3])
    if 0 == (header[0] - 16) :
      return ""
    res = self.sock.recv(header[0] - 16);
    return res

if __name__ == '__main__':
  client = RapidClient();
  client.connect("localhost", 6601)

  testdata="""method=ping"""

  print client.send_recv(testdata);