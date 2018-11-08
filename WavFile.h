#pragma once
#include<iostream>
#include<string>
#define RIFF_SIGN_ID 0x46464952ul 
#define WAVE_SIGN_ID 0x45564157ul 
#define FMT__SIGN_ID 0x20746D66ul
#define FACT_SIGN_ID 0x74636166ul 
#define DATA_SIGN_ID 0x61746164ul
enum FILE_STAT{FILE_OPEN_BAD,FILE_OPEN_FAIL,FILE_OPEN_EOF,FILE_OPEN_NORMAL};//�ļ��򿪺��״̬
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#ifndef WORD
typedef unsigned short WORD;
#endif
#ifndef BYTE
typedef unsigned char BYTE;
#endif
//������ͷ�ļ��ṹ44�ֽ�
struct HeaderType
{
	DWORD riff;                 /*RIFF����Դ�ļ�ͷ�� 4byte 0x46464952 'R' 'I' 'F' 'F'*/
	DWORD file_len;                /*�ļ�����4byte*/
	DWORD wave;                    /*"WAVE"��־4byte  WAV�ļ���־ 0x45564157 'W','A','V','E'*/
	DWORD fmt;                  /*"fmt"��־4byte ���θ�ʽ��־ 0x20746D66 'f','m','t',' '*/
	DWORD NI1;                     /*��ʽ�鳤��4byte*/
	WORD format_type;              /*��ʽ���(10HΪPCM��ʽ����������)2byte*/
	WORD Channels;                 /*Channels 1 = ������; 2 = ������2byte*/
	DWORD frequency;               /*����Ƶ��4byte*/
	DWORD trans_speed;             /*��Ƶ���ݴ�������4byte*/
	WORD dataBlock;                /*���ݿ�ĵ����������ֽ���ģ�2byte*/
	WORD sample_bits;              /*����������λ��(8/16) 2byte*/
	DWORD data;                 /*���ݱ�Ƿ�"data" 4byte ���ݱ�־�� 0x61746164, 'd','a','t','a' */
	DWORD wav_len; /*�������ݵĳ��� 4byte*/
	char * wav_data;
	HeaderType()
	{
		riff = 0;
		wave = 0;
		file_len = 0;
		fmt = 0;
		NI1 = 0;
		format_type = 0;
		Channels = 0;
		frequency = 0;
		frequency = 0;
		trans_speed = 0;
		dataBlock = 0;
		sample_bits = 0;
		data = 0;
		wav_len = 0;
		wav_data = NULL;
	}
};

//FACT��ļ��
struct UNKNOW_BLOCK{
	DWORD ID;  // δ֪�� 
	DWORD Size; // δ֪�鳤��
};
struct FACT_BLOCK{
	DWORD FactID;   // ��ѡ���ֱ�ʶ 0x74636166 'f','a','c','t' 
	DWORD FactSize; // ��ѡ���ֳ���
	BYTE  Data[1];  // ��ѡ�������� 
};
class WavFile
{
public:
	WavFile();
	~WavFile();
public:
	bool OpenWavFile(std::string audio_file);
	char* GetVioceData();
	DWORD GetChannels();
	DWORD GetFrequency();
	DWORD GetNI1();
	DWORD GetSample_bits();
	DWORD GetWav_len();
	bool SavePCMData(std::string file_name);
	bool SavePCMData(const char * file_name);
	size_t GetAudioHeadLen();
	static bool PCMTranceWav(const char* file_name, const char* outfile_name = "test.wav",  WORD channels = 2, DWORD sample_bits = 16, DWORD frequncy = 44100);
	static bool PCMTranceWav(const char* file_name, WORD channels = 2, DWORD sample_bits = 16, DWORD frequncy = 44100, const char* outfile_name = "test.wav");
	void close();
protected:
	static FILE_STAT Check_File_Stata(std::ios& fs);
private:
	bool m_file_open;
	HeaderType HT;
	size_t m_sAudioLen;
};

