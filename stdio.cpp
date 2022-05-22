#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>   
#include <stdlib.h>    
using namespace std;

char decimal[100];

int recursive_itoa(int arg) {
	int div = arg / 10;
	int mod = arg % 10;
	int index = 0;
	if (div > 0) { 
		index = recursive_itoa(div);
	}
	decimal[index] = mod + '0';
	return ++index;
}

char *itoa(const int arg) {
	bzero(decimal, 100);
	int order = recursive_itoa(arg);
	char *new_decimal = new char[order + 1];
	bcopy(decimal, new_decimal, order + 1);
	return new_decimal;
}

//DONE
//prints formatted output to stdout
int printf(const void *format, ...) {
	va_list list;
	va_start(list, format);

	char *msg = (char *)format;
	char buf[1024];
	int nWritten = 0;

	int i = 0, j = 0, k = 0;
	while (msg[i] != '\0') {
		if (msg[i] == '%' && msg[i + 1] == 'd')	{
			buf[j] = '\0';
			nWritten += write(1, buf, j);
			j = 0;
			i += 2;

			int int_val = va_arg(list, int);
			char *dec = itoa(abs(int_val));
			if (int_val < 0) {
				nWritten += write(1, "-", 1);
			}	
			nWritten += write(1, dec, strlen(dec));
			delete dec;
		}	else {
			buf[j++] = msg[i++];
		}	
	}
	if (j > 0) {
		nWritten += write(1, buf, j);
	}	
	va_end( list );
	return nWritten;
}

//DONE
//sets the size of an input/output stream buffer
int setvbuf(FILE *stream, char *buf, int mode, size_t size) {
	if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)	{
		return -1;
	}	
	stream->mode = mode;
	stream->pos = 0;
	if (stream->buffer != (char *)0 && stream->bufown == true) {
		delete stream->buffer;
	}
	
	switch ( mode ) {
		case _IONBF:
			stream->buffer = (char *)0;
			stream->size = 0;
			stream->bufown = false;
			break;
		case _IOLBF:
		case _IOFBF:
			if (buf != (char *)0) {
				stream->buffer = buf;
				stream->size   = size;
				stream->bufown = false;
			}	else {
				stream->buffer = new char[BUFSIZ];
				stream->size = BUFSIZ;
				stream->bufown = true;
			}
			break;
	}
	return 0;
}

//DONE
//sets the size of an input/output stream buffer
void setbuf(FILE *stream, char *buf) {
	setvbuf(stream, buf, ( buf != (char *)0 ) ? _IOFBF : _IONBF , BUFSIZ);
}


//DONE
//opens a file
FILE *fopen(const char *path, const char *mode) {
	FILE *stream = new FILE();
	setvbuf(stream, (char *)0, _IOFBF, BUFSIZ);
	
	// fopen( ) mode
	// r or rb = O_RDONLY
	// w or wb = O_WRONLY | O_CREAT | O_TRUNC
	// a or ab = O_WRONLY | O_CREAT | O_APPEND
	// r+ or rb+ or r+b = O_RDWR
	// w+ or wb+ or w+b = O_RDWR | O_CREAT | O_TRUNC
	// a+ or ab+ or a+b = O_RDWR | O_CREAT | O_APPEND

  switch(mode[0]) {
  case 'r':
	  if (mode[1] == '\0') {           // r
		  stream->flag = O_RDONLY;
	  }  else if ( mode[1] == 'b' ) {    
		  if (mode[2] == '\0') {         // rb
			  stream->flag = O_RDONLY;
		  } else if (mode[2] == '+') {      // rb+
			  stream->flag = O_RDWR;
		  }			  
	  }
	  else if (mode[1] == '+') {        // r+  r+b
		  stream->flag = O_RDWR;
	  }  
	  break;
  case 'w':
	  if (mode[1] == '\0') {           // w
		  stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
	  }	else if (mode[1] == 'b') {
		  if (mode[2] == '\0') {         // wb
			  stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
		  }	else if (mode[2] == '+') {     // wb+
			  stream->flag = O_RDWR | O_CREAT | O_TRUNC;
		  }	  
	  }
	  else if (mode[1] == '+') {       // w+  w+b
		  stream->flag = O_RDWR | O_CREAT | O_TRUNC;
	  }
	  break;
  case 'a':
	  if (mode[1] == '\0') {           // a
		  stream->flag = O_WRONLY | O_CREAT | O_APPEND;
	  } else if (mode[1] == 'b') {
		  if (mode[2] == '\0') {         // ab
			  stream->flag = O_WRONLY | O_CREAT | O_APPEND;
		  } else if (mode[2] == '+') {     // ab+
			  stream->flag = O_RDWR | O_CREAT | O_APPEND;
		  }	  
	  }
	  else if (mode[1] == '+') {        // a+  a+b
		  stream->flag = O_RDWR | O_CREAT | O_APPEND;
	  } 
	  break;
  }
  
  mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	//sets FILE fd, flag
  if ((stream->fd = open(path, stream->flag, open_mode)) == -1) {
	  delete stream;
	  printf("fopen failed\n");
	  stream = NULL;
  }
  
  return stream;
}


//clears an input/output stream buffer
int fpurge(FILE *stream) {
	stream->pos = 0;
	stream->actual_size = 0;
	return 0;
}

//syncronizes an output stream with the actual file
int fflush(FILE *stream) {
	if (stream->flag == O_RDONLY) {
		return -1;
	}
	int writeCount = write(stream->fd, &stream->buffer[stream->pos], stream->actual_size - stream->pos);
	if (writeCount == -1) {
		stream->eof = true;
		return -1;
	} else {
		return 0;
	}
}

//executes read() to refill FILE buffer
int readFromFile(FILE* stream) {
	int readCount = read(stream->fd, stream->buffer, stream->size); //returns actual size of FILE buffer after read
	stream->lastop = 'r';
	if (readCount != 0) {
		stream->actual_size = readCount;
		stream->pos = 0;
		stream->eof = false;
	} else {
		stream->eof = true;
	}
	return readCount;
}

//reads from a file
//ptr = This is the pointer to a block of memory with a minimum size of size*nmemb bytes.
//size = size in bytes of each element to be read
//nmemb = number of elements, each of "size" bytes
//stream = FILE object that specifies an input stream
//fread(buf, 1, 100, file_r)
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	// complete it
	if (stream->flag == (O_WRONLY | O_CREAT | O_TRUNC) || stream->flag == (O_WRONLY | O_CREAT | O_APPEND)) { //
		printf("ERROR: Invalid read permissions\n");
		return 0;
	}
	stream->lastop == 'r';

	//UNBUFFERED MODE
	if (stream->mode == _IONBF) {
		int readSize = read(stream->fd, ptr, nmemb);
		if (readSize < nmemb) {
			stream->eof = true;
		}
		return readSize;
	}
	
	int currPos = 0;
	while (currPos < nmemb * size) {
		int requestSize = nmemb * size - currPos; //amount of data left to fill
		int copySize = 0; //numbers of chars to be copied to output

		if (stream->pos == stream->actual_size || stream->lastop == 'w') {
			if (readFromFile(stream) == 0) {; //get fresh FILE buffer
				 stream->eof = true;
				 break; //exit loop when at EOF
			}
		}

		void* readPtr = stream->buffer + stream->pos;
		if (requestSize < stream->actual_size - stream->pos) { //request is less than what is in FILE buffer
			copySize = requestSize;
			stream->pos += requestSize; //move pos
		} else {
			copySize = stream->actual_size - stream->pos; //copy all remaining data in FILE buffer
			stream->pos = stream->actual_size; //move pos to end
		}

		memcpy(ptr, readPtr, copySize);

		ptr += copySize; //move ptr in the output buffer
		currPos += copySize;
	}	
	
	return currPos;
}

int writeToFile(FILE* stream) {
	int writeCount = write(stream->fd, stream->buffer, stream->actual_size);
	stream->lastop = 'w';
	if (writeCount != 0) {
		fpurge(stream);
		stream->eof = false;
	} else {
		stream->eof = true;
	}
	return writeCount;
}

//writes to a file
//ptr = pointer to the array of elements to be written
//size = size in bytes of each element to be read
//nmemb = number of elements, each of "size" bytes
//stream = FILE object that specifies an output stream
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	if (stream->flag == O_RDONLY) {
		printf("ERROR: Invalid write permissions\n");
		return 0;
	}
	stream->lastop == 'w';

	//UNBUFFERED MODE
	if (stream->mode == _IONBF) {
		return write(stream->fd, ptr, nmemb * size);
	}

	if (stream->lastop == 'r') { //reset FILE buffer pos
		fpurge(stream);
	}

	int currPos = 0; //current position in the input
	//bool endOfInput = false;
	while (currPos < nmemb * size) {
		int inputSize = nmemb * size - currPos; //amount of data left to write
		int writeSize = 0;

		void* writePtr = stream->buffer + stream->pos;
		if (inputSize < stream->size - stream->pos) { //input is less than available space in FILE buffer
			writeSize = inputSize;
		} else {
			writeSize = stream->size - stream->pos; //fill remaining space in FILE buffer
		}

		memcpy(writePtr, ptr, writeSize);
		stream->pos += writeSize; //move pos
		stream->actual_size += writeSize;

		if (writeToFile(stream) == 0) {; //write FILE buffer to file of actual_size
				stream->eof = true;
				break; //exit loop when at EOF
		}

		fpurge(stream);

		ptr += writeSize; //move ptr in the input buffer
		currPos += writeSize;	
	}
	return currPos;
}

//reads a character from a file stream
int fgetc(FILE *stream) {
	char c;
	int readCount = fread(&c, 1, 1, stream);
	if (readCount == 0) { //reached end of file
		stream->eof = true;
		c = EOF;
	}
	return c;
}

//writes a character to a file stream
int fputc(int c, FILE *stream) {
	// complete it
	int writeCount = fwrite(&c, 1, 1, stream);
	if (writeCount == 0) {
		stream->eof = true;
	}
	return writeCount;
}

//reads a line from a file stream to a buffer
char *fgets(char *str, int size, FILE *stream) {
	if (stream->flag == (O_WRONLY | O_CREAT | O_TRUNC) || stream->flag == (O_WRONLY | O_CREAT | O_APPEND)) { //
		printf("ERROR: Invalid read permissions\n");
		return NULL;
	}
	
	//UNBUFFERED MODE
	if (stream->mode == _IONBF) {
		stream->lastop == 'r';
		if (stream->eof) {
			return NULL;
		}
		int endPos = 0;
		for (int i = 0; i < size - 1; i++) { //from 0 to 99
			int readCount = read(stream->fd, &str[i], 1);
			if (readCount <= 0) { //reached end of file
				stream->eof = true;
				break;
			}
			endPos++;
			if (str[i] == '\n') { //reached end of line
				break;
			}
		}
		str[endPos] = '\0'; //append '\0' at the end of the buffer line
		return str;
	}


	int currPos = 0; //current position in the output buffer
	bool isLineEnd = false;
	while (currPos < size - 1 && !stream->eof && !isLineEnd) {
		int requestSize = size - 1 - currPos; //amount of data left to fill. Leave room for /0 character at the end of output buffer
		//char* readPtr = stream->buffer + stream->pos;
		int copySize = 0; //numbers of chars to be copied to output

		if (stream->pos == stream->actual_size || stream->lastop == 'w') {
			if (readFromFile(stream) == 0) {; //get fresh FILE buffer
				 break; //exit loop when at EOF
			}
		}

		if (requestSize < stream->actual_size - stream->pos) { //request is less than what is in FILE buffer
			copySize = requestSize;
		} else {
			copySize = stream->actual_size - stream->pos; //copy all remaining data in FILE buffer
		}

		for (int i = 0; i < copySize; i++) {
			*str = stream->buffer[stream->pos];
			
			currPos++; //move current position in the output buffer
			stream->pos++; //move the FILE pos
			if (*str == '\n') { //reached end of line
				str++; //move output pointer
				isLineEnd = true;
				break;
			}
			str++; //move output pointer
		}
	}	

	if (stream->eof) {
		return NULL;
	}
	*str = '\0'; //append '\0' at the end of the buffer line
	return str;
}

//writes a character string to a file stream
int fputs(const char *str, FILE *stream) {
	if (stream->flag == O_RDONLY) {
		printf("ERROR: Invalid write permissions\n");
		return 0;
	}

	if (stream->mode == _IONBF) {
		stream->lastop == 'w';
		int lineSize = 0;
		char c = str[0];
		while (c != '\0' && c != EOF) {
			lineSize++;
			if (c == '\n') {
				break;
			}
			c = str[lineSize];
		}
		int writeCount = write(stream->fd, str, lineSize);
		return writeCount;
	}

	if (stream->lastop == 'r') { //reset FILE buffer pos
		fpurge(stream);
	}

	int currPos = 0; //current position in the input
	bool isEndLine = false;
	while (!isEndLine) {
		int freeSize = stream->size - stream->pos; //available space in FILE buffer
		int writeSize = 0;
		for (int i = stream->pos; i < stream->size; i++) {
			stream->buffer[i] = str[currPos];
			writeSize++;
			if (str[currPos] == '\n' || str[currPos] == '\0' || str[currPos] == EOF) {
				isEndLine = true;
				currPos++;
				break;
			}
			currPos++;
		}

		stream->pos += writeSize; //move pos
		stream->actual_size += writeSize;

		if (writeToFile(stream) == 0) {; //write FILE buffer to file of actual_size
				stream->eof = true;
				break; //exit loop when at EOF
		}

		fpurge(stream);
	}

	stream->lastop == 'w';
	return currPos;
}

//DONE
//checks for end-of-file
int feof(FILE *stream) {
	return stream->eof == true;
}

//moves the file position to a specific location in a file
int fseek(FILE *stream, long offset, int whence) {
	int ret;
	if (stream->mode != _IONBF && whence == SEEK_CUR && stream->pos + offset >= 0 && stream->pos + offset < stream->actual_size) {  //from middle of fd
			stream->pos += offset;
			ret = offset;
			stream->eof = false;
	} else { //from beginning of fd
		ret = lseek(stream->fd, offset, whence);
		if (ret != -1) {
			readFromFile(stream);
		}
	}
	return ret;
}

//closes a file
int fclose(FILE *stream) {
	if (stream->mode != _IONBF) {
		fflush(stream);
		fpurge(stream);
	}
	return close(stream->fd);
}