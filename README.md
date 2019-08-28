<h1>steganography</h1>

File-format: WAV

Sctructure: <p>
![pic](https://i.stack.imgur.com/NEYC2.gif)

For this issue we use LSB algorithm. Need to take BitsPerSample value to know the maximum density value. We alter the data block wich is a sequence of audio-samples. 
LSB algorithm (https://pdfs.semanticscholar.org/5051/8f90cd3af38a44ee74262ab8217566bf33f5.pdf).

message to write structure:
density value, mark, data size, filename, data.

