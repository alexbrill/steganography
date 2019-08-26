#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <conio.h>

using namespace std;

typedef unsigned int uint;
typedef vector<unsigned char> byte_array;

const uint BYTE_SIZE = 8;
const uint UINT_SIZE = 32;

const uint N_DATA_FIELD = 40;
const uint N_DATA       = 44;
const uint N_BPS_FIELD  = 34;
const uint N_FILE_SIZE  = 4;
const uint N_EXTENSION  = 8;

const uint L_DATA_FIELD = 4;
const uint L_BPS_FIELD  = 2;
const uint L_EXTENSION  = 5;
const uint L_FILE_SIZE  = 4;
const uint L_MARK 		= 4;
const uint L_DENSITY_OF_FILE = 5;

const uint L_MSG_CFG = 28;
const uint MARK = 111111;



union
{
  uint INT;
  unsigned char CHAR[4];
} int2byte;



void Steganography(uint, byte_array, const byte_array&, uint, uint);
void showMenu(bool);
void densityTest(uint, uint, uint);
void getFileData(byte_array&, const string&);
uint getNBytes(const byte_array&, int, int);
string getFileName(const string&);
bool isFileExist(const string&);
bool isPowerOfTwo(uint);


int main() 
{
	bool notDone = true,
		 contExist = false,
		 fileExist = false;
	
	string fileContPath,
		   filePath;
		   
	byte_array container,
			   data;
			   
	uint dataFieldSize = 0,
		 bitsPerSample = 0,
		 sampleSizeInBytes = 0,
		 dataSize = 0,
		 density = 0;
	
	
	while(notDone)
	{
		showMenu(contExist && fileExist);
		
		switch(_getch())
		{
			case '1':
				cout << "Path to the container: ";
				getline(std::cin, fileContPath);				
				contExist = isFileExist(fileContPath);
				
				if (contExist)
				{
					container.clear();
					getFileData(container, fileContPath);
					
					dataFieldSize = getNBytes(container, N_DATA_FIELD, L_DATA_FIELD);		
					bitsPerSample = getNBytes(container, N_BPS_FIELD, L_BPS_FIELD);
					sampleSizeInBytes = bitsPerSample / BYTE_SIZE;					
				}	
										
				break;
				
			case '2':
				cout << "Path to the file: ";
				getline(std::cin, filePath);				
				fileExist = isFileExist(filePath);
				
				if (fileExist)
				{
					data.clear();
					data.resize(L_MSG_CFG);
					getFileData(data, filePath);
					
					int2byte.INT = MARK;					
					for (uint i = 0; i < L_MARK; ++i) data[i] = int2byte.CHAR[i];
					
					dataSize = data.size();
					int2byte.INT = dataSize + L_DENSITY_OF_FILE;
					for (uint i = 0; i < L_FILE_SIZE; ++i) data[N_FILE_SIZE + i] = int2byte.CHAR[i];
						
					string fileName = getFileName(filePath);
					uint ind = N_EXTENSION;
					for (char c : fileName) data[ind++ % 28] = c;					
				}
				
				break;
				
			case '3':
				if (!(contExist && fileExist)) break;
				
				cout << "Density test:" << endl;
				densityTest(dataSize, dataFieldSize, bitsPerSample);
				
				break;
				
			case '4':
				if (!(contExist && fileExist)) break;
				
				cout << "Steganography." << endl
				     << "Density: ";
				cin >> density;
				
				if (density <= bitsPerSample && density > 0 && isPowerOfTwo(density))
				{
					if ((dataFieldSize * density) >= \
		 					(dataSize + L_MSG_CFG + L_DENSITY_OF_FILE) * bitsPerSample)
		 			{
		 				Steganography(density, container, data, sampleSizeInBytes, dataSize);		
					}
					else cout << "Too big. Try to minimize a density." << endl;
				}
				else cout << "Density is out of range." << endl;
				
				break;
				
			case '0':
				notDone = false;
				
				break;
		}
		
	}
	
	
	system("pause");
	
	return 0;
}

bool isFileExist(const string& path)
{
	bool isExist = std::ifstream(path) != nullptr;
	cout << (isExist? "Done." : "This file isn't exist") << endl;
	return isExist;
}

void showMenu(bool ready) 
{
	system("pause");
	system("cls");
	
	cout << "MENU:" << endl
		 << "1 - enter path to the container" << endl
		 << "2 - enter path to the file that will be embedded" << endl;
		 
	if (ready)	 
	cout << "3 - density test" << endl
		 << "4 - make steganography" << endl;
		 
	cout << "0 - exit" << endl
		 << "_______________________" << endl
		 << endl << endl;
}

void densityTest(uint dataSize, uint contSize, uint BPS)
{
	uint temp = (dataSize + L_MSG_CFG + L_DENSITY_OF_FILE) * BPS;
	
	for (uint d = 1; d <= BPS; d <<= 1)
	{
		uint bytesUsed = temp / d;					
									
		cout << d << " : " 	<< setprecision(2) << bytesUsed / float(contSize) * 100 << "%" 
				  << " (" << bytesUsed << " samples used of " << contSize << ")" << endl;						
	}
}

uint getNBytes(const byte_array &data, int start, int N)
{
	uint numb = 0;	
	for (uint i = 0; i < N; ++i)
		numb |= (uint(data[start + i]) << (i * BYTE_SIZE));		
	return numb;
}

void getFileData(byte_array &data, const string& path)
{
	ifstream source(path, ios::binary);	
	while(!source.eof())
		data.push_back(source.get());	
	source.close();
}

string getFileName(const string& s)
{
   char sep = '/';
#ifdef _WIN32
   sep = '\\';
#endif

   size_t i = s.rfind(sep, s.length());
   if (i != string::npos) {
      return(s.substr(i+1, s.length() - i));
   }

   return s;
}

bool isPowerOfTwo(uint numb)
{
	return numb && !(numb & (numb - 1));
}

void Steganography(uint density, byte_array container, const byte_array &data, uint sampleSizeInBytes, uint dataSize)
{
	uint ind_d = 0, 
		 ind_c = 0,
		 extra_ind_c = 0,
		 shift_d = 0,
		 shift_c = 0,
		 extra_shift_c = 0;
	
	//write density in container	
	for (uint i = 0; i < L_DENSITY_OF_FILE; ++i)
		if ((density - 1) & (1 << i)) container[N_DATA + i * sampleSizeInBytes] |= 1;
		else  container[N_DATA + i * sampleSizeInBytes] &= (~1);	
	
	while (ind_d < dataSize)
	{
		uint temp = (data[ind_d] & (1 << shift_d)) ? 1 : 0;	
		
		if (temp) container[ind_c + extra_ind_c + N_DATA + L_DENSITY_OF_FILE * sampleSizeInBytes] |= (1 << shift_c);
		else
		{
			temp = ~(1 << shift_c);
			container[ind_c + extra_ind_c + N_DATA + L_DENSITY_OF_FILE * sampleSizeInBytes] &= temp;
		}			
		
		shift_d = (shift_d + 1) % BYTE_SIZE;
		extra_shift_c = (extra_shift_c + 1) % density;
		extra_ind_c = extra_shift_c / BYTE_SIZE;
		shift_c = extra_shift_c % BYTE_SIZE;						
		
		if (shift_d == 0) ++ind_d;
		if (extra_shift_c == 0) ind_c += sampleSizeInBytes;
	}
		
	ofstream resultFile("result.wav", ios::binary);	
	for (uint i = 0; i < container.size(); ++i)
		resultFile.write((char*)&container[i], sizeof(char));	
	resultFile.close();
	
	cout << "Done." << endl;
}
