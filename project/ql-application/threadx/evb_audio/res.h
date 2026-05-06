#ifndef _RES_H_
#define _RES_H_

typedef struct Download_header {
uint32_t magic_number; /* magic number ħ�� YMZN*/
uint32_t version; /* version: 0.0.0.0 �ļ��汾��*/
uint16_t header_chksum; /* header checksum ͷУ���   (64���ֽ� chksum)*/
uint16_t data_chksum; /* data checksum ������У��ֵ ���ļ�����У��)*/
uint32_t data_size; /* data size �ļ����ȣ�����ͷ��*/
uint16_t attribute; /* attribute  0ΪAPP��1Ϊ������Դ��2ΪTTS�⣬3ΪOTA APP*/
uint16_t index; /*��Ŀ�ڽ��洢λ�������š�Ĭ��ΪFFFF����ʱ���ļ�������*/
int next_addr; /*��һ�ļ�Ѱַ����Ե�ǰͷ����ƫ������Ϊ0��ǰ�ļ����޺����ļ���������������У���������Ϣ�����Կ��ܴ��ڵ���ͷ���������ļ��峤��֮�͡�*/ 
uint32_t Resv[2]; /* section ID ����*/
uint32_t FileName[8]; /* private data �ļ�����������ΪFFFFʱ����*/
}download_header_t;


#define FILE_HEADER 0
#define FILE_DATA  1

#define	RESPRG	0
#define	RESWAV	1

struct rev_status {
	uint8_t revDataStatus;
    uint8_t revLen;
	uint32_t offset;
	uint32_t writeAddr;
	uint32_t BaseAddr;
	uint16_t checkSum;
};	

#define HEADER_SIZE  sizeof(struct Download_header)

const char *  getImgDataAddr(char const * pfileName);

extern int getFileDataAddrFromTable(char const *content,uint32_t *fileLen);

extern uint32_t getFileDataFromFlash(uint32_t addr,char *data,uint32_t len);

extern int getFileDataPathFromFlash(char const *content,char *buff, uint32_t buffsize);

extern struct rev_status rev_s;
extern struct Download_header download_header;
extern int IdxRangChk(void);
extern int ResTabDefChk(void);
extern uint32_t getResVer(void);
extern int getResCount(uint16_t type);
extern int check_wav_file( char const *gbk_content, char *buff, int bufsize );

#endif // _YM_RESOURCE_H_
