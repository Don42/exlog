CC= 		gcc
CXX= 		g++
CFLAGS+= 	-I/usr/include
CFLAGS+=	-std=c99
CFLAGS+=	-g
LDFLAGS+=	-L/usr/lib

BIN=	exlog
OBJ+=	exlog.o

all:	$(BIN)

$(BIN):	$(OBJ)
	$(CC) $(LDFLAGS) $(LDADD) -o $@ $(OBJ)

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(BIN)
