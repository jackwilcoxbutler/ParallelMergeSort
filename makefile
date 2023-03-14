CC       = g++
CFLAGS   = -Wall -g
LDFLAGS  = -lreadline
OBJFILES = testing.o
TARGET   = testing

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~
