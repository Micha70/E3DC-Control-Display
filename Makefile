CXX=g++
ROOT_VALUE=E3DC-Control-Display

LIBS = -lbcm2835

all: $(ROOT_VALUE)

$(ROOT_VALUE): clean
	$(CXX) -O3 RscpExampleMain.cpp RscpProtocol.cpp AES.cpp SocketConnection.cpp fb_display.cpp framebuffer.cpp font_8x8.cpp io_BCM2835.cpp Prognose.cpp $(LIBS) -o $@


clean:
	-rm $(ROOT_VALUE) $(VECTOR)
