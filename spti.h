/**************************************************************************************************/
/*																							      */
/*					Copyright (c) 2018-2024 by Hosin Global Electronics Co., LTD.				  */
/*																								  */
/**************************************************************************************************/

#include <vector> 
#include <string>
#include <iostream>
#include <queue>
#include <windows.h>
#include <devioctl.h>
#include <ctime>
#include <ntddscsi.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <chrono>
#include <sstream>
#include <locale.h>
#include <codecvt>
#include <stdexcept>
#include <iomanip> // �������������ʽ
#include "Version.h"
#include <thread>

using namespace std;

//=================================================================================================
//										Macro Define
//=================================================================================================

#ifndef __SPTI_H__
#define __SPTI_H__

#define CDB6GENERIC_LENGTH														(6)
#define CDB10GENERIC_LENGTH														(10)
#define CDB12GENERIC_LENGTH														(12)
#define CDB16GENERIC_LENGTH														(16)

#define SCSIOP_TEST_UNIT_READY													(0x00)
#define SCSIOP_REZERO_UNIT														(0x01)
#define SCSIOP_REWIND															(0x01)
#define SCSIOP_REQUEST_BLOCK_ADDR												(0x02)
#define SCSIOP_REQUEST_SENSE													(0x03)
#define SCSIOP_FORMAT_UNIT														(0x04)
#define SCSIOP_READ_BLOCK_LIMITS												(0x05)
#define SCSIOP_REASSIGN_BLOCKS													(0x07)
#define SCSIOP_READ6															(0x08)
#define SCSIOP_RECEIVE															(0x08)
#define SCSIOP_WRITE6															(0x0A)
#define SCSIOP_PRINT															(0x0A)
#define SCSIOP_SEND																(0x0A)
#define SCSIOP_SEEK6															(0x0B)
#define SCSIOP_TRACK_SELECT														(0x0B)
#define SCSIOP_SLEW_PRINT														(0x0B)
#define SCSIOP_SEEK_BLOCK														(0x0C)
#define SCSIOP_PARTITION														(0x0D)
#define SCSIOP_READ_REVERSE														(0x0F)
#define SCSIOP_WRITE_FILEMARKS													(0x10)
#define SCSIOP_FLUSH_BUFFER														(0x10)
#define SCSIOP_SPACE															(0x11)
#define SCSIOP_INQUIRY															(0x12)
#define SCSIOP_VERIFY6															(0x13)
#define SCSIOP_RECOVER_BUF_DATA													(0x14)
#define SCSIOP_MODE_SELECT														(0x15)
#define SCSIOP_RESERVE_UNIT														(0x16)
#define SCSIOP_RELEASE_UNIT														(0x17)
#define SCSIOP_COPY																(0x18)
#define SCSIOP_ERASE															(0x19)
#define SCSIOP_MODE_SENSE														(0x1A)
#define SCSIOP_START_STOP_UNIT													(0x1B)
#define SCSIOP_STOP_PRINT														(0x1B)
#define SCSIOP_LOAD_UNLOAD														(0x1B)
#define SCSIOP_RECEIVE_DIAGNOSTIC												(0x1C)
#define SCSIOP_SEND_DIAGNOSTIC													(0x1D)
#define SCSIOP_MEDIUM_REMOVAL													(0x1E)
#define SCSIOP_READ_CAPACITY													(0x25)
#define SCSIOP_READ																(0x28)
#define SCSIOP_WRITE															(0x2A)
#define SCSIOP_SEEK																(0x2B)
#define SCSIOP_LOCATE															(0x2B)
#define SCSIOP_WRITE_VERIFY														(0x2E)
#define SCSIOP_VERIFY															(0x2F)
#define SCSIOP_SEARCH_DATA_HIGH													(0x30)
#define SCSIOP_SEARCH_DATA_EQUAL												(0x31)
#define SCSIOP_SEARCH_DATA_LOW													(0x32)
#define SCSIOP_SET_LIMITS														(0x33)
#define SCSIOP_READ_POSITION													(0x34)
#define SCSIOP_SYNCHRONIZE_CACHE												(0x35)
#define SCSIOP_COMPARE															(0x39)
#define SCSIOP_COPY_COMPARE														(0x3A)
#define SCSIOP_WRITE_DATA_BUFF													(0x3B)
#define SCSIOP_READ_DATA_BUFF													(0x3C)
#define SCSIOP_CHANGE_DEFINITION												(0x40)
#define SCSIOP_READ_SUB_CHANNEL													(0x42)
#define SCSIOP_READ_TOC															(0x43)
#define SCSIOP_READ_HEADER														(0x44)
#define SCSIOP_PLAY_AUDIO														(0x45)
#define SCSIOP_PLAY_AUDIO_MSF													(0x47)
#define SCSIOP_PLAY_TRACK_INDEX													(0x48)
#define SCSIOP_PLAY_TRACK_RELATIVE												(0x49)
#define SCSIOP_PAUSE_RESUME														(0x4B)
#define SCSIOP_LOG_SELECT														(0x4C)
#define SCSIOP_LOG_SENSE														(0x4D)


#define Partition_Table_Start_Index                                             (446)
#define BPB_Start_Index                                                         (11)
#define DEBUG_Flag                                                              (0)
//=================================================================================================
//											Structure
//=================================================================================================

typedef unsigned char				UBYTE;       
typedef unsigned short				UWORD;
typedef long						LONG;  // ��һЩ��������4Byte��������һЩ��������8Byte  
typedef unsigned long				ULONG;       
typedef unsigned long long          ULONGLONG;

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
    SCSI_PASS_THROUGH				spt;
    ULONG							Filler;      
    UCHAR							ucSenseBuf[32];
    UCHAR							ucDataBuf[512];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
    SCSI_PASS_THROUGH_DIRECT		sptd;
    ULONG							Filler;      
    UCHAR							ucSenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

typedef struct _FLASH_CAPACITY {
	ULONG							ulMaxLBA;
	ULONG							ulPageSize;
} FLASH_CAPACITY, *PFLASH_CAPACITY;


//=================================================================================================
//										Function Declaration
//=================================================================================================

BOOL	IssueCDB(HANDLE fileHandle, UCHAR ucCDBLength, PUCHAR CDBData, UCHAR ucDataIn, PVOID pDataBuf, ULONG usBufLen);
UCHAR	UFD_READ(HANDLE fileHandle, ULONG ulLBA, USHORT usLen, PUCHAR pucBuffer, UCHAR ucLUN = 0);
UCHAR	UFD_WRITE(HANDLE fileHandle, ULONG ulLBA, USHORT usLen, PUCHAR pucBuffer, UCHAR ucLUN = 0);

//=================================================================================================
//										  User New
//=================================================================================================

struct Partition
{
    UCHAR BootIndicator;           // ������־����ʾ�÷����Ƿ��ǻ������0x80 ��ʾ���0x00 ��ʾ�ǻ����
    UCHAR StartHead;               // ��ʼ��ͷ�ţ���ʾ��������ʼλ�á�
    UWORD StartSector;          // ��ʼ�����ź�����ţ���ʾ��������ʼλ�ã�����ź������������һ�𣩡�
    UCHAR PartitionTypeIndicator;  // �������ͱ�־����ʾ�÷������ļ�ϵͳ���ͣ��� 0x07 ��ʾ NTFS����
    UCHAR EndHead;                 // ������ͷ�ţ���ʾ�����Ľ���λ�á�
    UWORD EnDSector;            // ���������ź�����ţ���ʾ�����Ľ���λ�ã�����ź������������һ�𣩡�
    ULONG RelativeSector;          // ��������ţ���ʾ�Ӵ��̵ĵ�һ���������÷�����ʼ������ƫ������ռ�� 4 �ֽڣ���
    ULONG SectorsInPartition;   // ���������������������ڱ�ʾ�����Ĵ�С��ռ�� 4 �ֽڣ���
};

struct MBR
{
    Partition Partitions[1];        // ��ʾ��ǰ��1������
};

struct BPB
{
    UWORD BytesPersector;            // ÿ�������ֽ�����ͨ��Ϊ512�ֽڻ�1024�ֽڵȡ�
    UCHAR SectorsPerCluster;         // ÿ���ص���������һ�����ɶ��������ɣ����ļ�ϵͳ����С���䵥λ��
    UWORD ReservedSectors;           // ������������ͨ��ָ�Ӿ����ʼλ�õ���һ��FAT��֮�����������
    UCHAR NumberOfFATs;              // FAT��ĸ�����ͨ��Ϊ2�����������ౣ����
    UWORD RootEntries;               // ��Ŀ¼������Ŀ������ʾ��Ŀ¼����������ɵ��ļ�����Ŀ¼������
    UWORD SectorsOnSmallVolumes;     // �����������������С��65535��ʹ������ֶΣ�����ʹ��SectorsOnLargeVolumes�ֶΡ�
    UCHAR MediaDescriptor;           // ���������������ڱ�ʶ�������ͣ�����Ӳ�̡����̵ȡ�
    UWORD SectorsPerFATOnSmallVolumes; // ÿ��FAT��ռ�õ������������С����FAT12��FAT16����
    UWORD SectorsPerTrack;           // ÿ�ŵ�����������ͨ������CHSѰַģʽ��
    UWORD Heads;                     // ��ͷ����ͨ������CHSѰַģʽ��
    ULONG HiddenSectors;             // �������������Ӿ����ʼλ�õ�ʵ�����������ƫ�����������ڷ�������������
    ULONG SectorsOnLargeVolumes;     // ����������������Դ����FAT32����󣩡�

    ULONG SectorsPerFAT;             // ÿ��FATռ�õ�����������Դ����FAT32����
    UWORD ExtendedFlags;             // ��չ��־��ͨ������FAT32�еľ����������ǡ�
    UWORD Version;                   // �ļ�ϵͳ�汾�ţ�ͨ����FAT32�İ汾��Ϣ��
    ULONG RootDir1stCluster;         // ��Ŀ¼����ʼ�غţ���ʾ��Ŀ¼���������е�λ�ã�������FAT32����
    UWORD FSInfoSector;              // �ļ�ϵͳ��Ϣ�����ţ���ʾ�ļ�ϵͳ��Ϣ������������š�
    UWORD BackupBootSector;          // BootSector�ı��������ţ���ʾ����������������������š�

    ULONG size;
};


struct DBR
{
    ULONG JMP;                      // JMP instruction(��תָ��,������ִ��������ת����������) 
    ULONGLONG OME;                  // OEM, �������ɴ������ļ�ϵͳ��OEM���̾��尲��
    BPB bpb;
};

struct File {
    vector<UCHAR> FileName = {};        // �ļ���
    wstring FileNameString = L"";        // �ַ������͵��ļ���

    vector<UCHAR> Extension = {};       // ��չ�� (3�ֽ�)
    wstring ExtensionString = L"";       // �ַ������͵���չ��

    UCHAR Attribution = 0;              // �ļ����� (1�ֽ�)
    UCHAR Reserved = 0;                 // ����λ (1�ֽ�)
    UCHAR CR = 0;                       // ����ʱ������� (1�ֽ�, ��ȷ��2��)

    vector<UCHAR> CreationTime = {};   // ����ʱ�� (2�ֽ�, HHMM��ʽ)
    vector<UCHAR> CreationDate = {};   // �������� (2�ֽ�, YYYYMMDD��ʽ)
    string CreationTimeString = "";   // �ַ������͵Ĵ���ʱ��

    vector<UCHAR> AccessDate = {};     // ���������� (2�ֽ�, YYYYMMDD��ʽ)
    string AccessDateString = "";   // �ַ������͵Ĵ���ʱ��
    ULONG ClusterNo = 0;                // ��ʼ�غ� (4�ֽ�)

    vector<UCHAR> ModifyTime = {};     // ����޸�ʱ�� (2�ֽ�, HHMM��ʽ)
    vector<UCHAR> ModifyDate = {};     // ����޸����� (2�ֽ�, YYYYMMDD��ʽ)
    string ModifyTimeString = "";     // �ַ������͵��޸�ʱ��

    vector<UCHAR> Size = {};            // �ļ���С (4�ֽ�)
    string SizeString = "";            // �ַ������͵��ļ���С

    // �������ú�������������ֶ�
    void reset() {
        FileName.clear();
        FileNameString.clear();

        Extension.clear();
        ExtensionString.clear();

        Attribution = 0;
        Reserved = 0;
        CR = 0;

        CreationTime.clear();
        CreationDate.clear();
        CreationTimeString.clear();

        AccessDate.clear();
        AccessDateString.clear();
        ClusterNo = 0;

        ModifyTime.clear();
        ModifyDate.clear();
        ModifyTimeString.clear();

        Size.clear();
        SizeString.clear();
    }
};

// ʹ�����ṹ�������ļ�Ŀ¼�ṹ
struct TreeNode {
    File FileInfo;
    vector<TreeNode*> Children;
    TreeNode* ParentNode = NULL; //���ڵ�
    bool IsFolderFlag; //��ǵ�ǰ�ڵ��Ƿ�Ϊ�ļ��С�1���ļ��У�0:�ļ�

    TreeNode(bool isFolder) : IsFolderFlag(isFolder) {} // ���캯������ʼ���ڵ�
};

// FAT32
class FAT32 {
public:
    // FAT32�еĽṹ
    MBR* mbr = new MBR();             // ��������¼��Master Boot Record��
    DBR* dbr = new DBR();             // DOS������¼��Disk Boot Record����Ҳ��Ϊ��������¼
    TreeNode* Root = new TreeNode(1);       // ��Ŀ¼�ڵ㣬��ʾ�ļ�ϵͳ�ĸ�Ŀ¼
       // �ļ����
    vector<vector<string>> FileAbsolutePath; //ÿ���ļ������ļ��еľ���·��

    // FAT32�и������ֵ���ʼ��ַ
    ULONG mbrStartAddr;  // MBR����ʼ��ַ
    ULONG dbrStartAddr;  // DBR����ʼ��ַ
    ULONG FATStartAddr1;  // FAT�����ʼ��ַ
    ULONG FATStartAddr2;  // FAT��2����ʼ��ַ
    ULONG DataRegionAddr; // ����������ʼ��ַ

    //FAT32���캯��
    FAT32(HANDLE FAT32FileHandle, wstring Name) {
        // ��ʼ�����ڵ�
        Root->FileInfo.FileNameString = Name;
        Root->FileInfo.ClusterNo = 2;

        // ��ʼ��mbr��ʼ��ַ
        mbrStartAddr = 0;

        // ����MBR
        ParsingMBR(FAT32FileHandle);

        // ��ʼ��dbr��ʼ��ַ
        dbrStartAddr = mbr->Partitions[0].RelativeSector;

        // ����DBR
        ParsingDBR(FAT32FileHandle);

        //��ȡFAT��1��FAT��2�Ŀ�ʼ��ַ
        FATStartAddr1 = dbrStartAddr + dbr->bpb.ReservedSectors;
        FATStartAddr2 = FATStartAddr1 + dbr->bpb.SectorsPerFAT;
        

        // ��ȡDATA����Ŀ�ʼ��ַ
        DataRegionAddr = dbrStartAddr + dbr->bpb.ReservedSectors + (dbr->bpb.SectorsPerFAT << 1);

        //��������Ŀ¼�ṹ
        CreateCataloge(FAT32FileHandle);

        ////��ȡÿ���ļ������ļ��еľ���·��
        //TraverseTree(Root, Root->FileInfo.FileNameString, Root->FileInfo.ClusterNo, FileAbsolutePath);

        // ��ӡ����Ŀ¼�ṹ
        cout << "<------------------------------------------Ŀ¼�ṹ------------------------------------------->" << endl;
        cout << "Create Times       " << '\t' << "Last Change Times   " << '\t' << "Access Times   " << '\t' << "Type          " << '\t' << "File Size"<< '\t' << "File Name" << endl;
        PrintDirectory(Root, 0);
        printf("\n");
    }

    // FAT32�еĳ�Ա����
    void ParsingMBR(HANDLE FfileHandle);
    void ParsingDBR(HANDLE FfileHandle);
    void ParsingDir(HANDLE FfileHandle, ULONG DataBaseAddress, TreeNode* Node);

    void CreateCataloge(HANDLE FfileHandle); // ʹ�����ṹ����������Ŀ¼
    void PrintDirectory(TreeNode* Node, int level); //��ӡ����Ŀ¼�ṹ

    TreeNode* FindNode(string FilePath); //Ѱ��ĳ���ļ������ļ������ڽڵ�
    ULONG DeleteFileOrDirInCluster(HANDLE FfileHandle, string DeleteFileName, TreeNode* ParentNode);//ɾ�������ļ�����Ŀ¼
    void DeleteFileClusterFromFAT(HANDLE FfileHandle, vector<ULONG> Clusters);//ɾ��FAT���ж�Ӧ�Ĵ�

    ULONG FAT32::FindFreeCluster(HANDLE FfileHandle);//��FAT����Ѱ�ҿ��еĴ�
    vector<ULONG> FindFileClusterFromFAT(HANDLE FfileHandle, ULONG clusternos);//��FAT����Ѱ���ļ������д�
    vector<ULONG> FindFreeClusterFromFATForCopy(HANDLE FfileHandle, ULONG ClusterNumber);// ��FAT���ҵ��ܹ�����ļ��Ĵز��޸ı�־
    vector<UCHAR> FindFileOrDirCode(HANDLE FfileHandle, string Name, TreeNode* parentNode); // �ҵ��ļ������ļ��ж�Ӧ����Ϣ�Լ��ڸ�Ŀ¼�µı���
    void CopySourceCodeToDestination(HANDLE FfileHandle, string FileName, TreeNode* DestinationNode, vector<UCHAR> SourceCode, ULONG SatrtCluster);// ����Դ�ļ�Code��Ŀ��·����
    void CopySourceFileContentToDestination(HANDLE FfileHandle, vector<ULONG> SourceClusters, vector<ULONG> DestinationClusters); // ����Դ�ļ����ݵ�Ŀ��·����
    void RecoverFile(HANDLE FfileHandle, string file_path);//�ָ�ɾ����ͼƬ
    
    File FindFileNameForRecover(HANDLE FfileHandle, TreeNode* parentnode, string filename);// �ڸ�Ŀ¼��Ѱ���ļ���
    void ModifyClusterInfo(HANDLE FfileHandle, ULONG number, ULONG next_cluster,int is_add_flag); // �޸ķ�������Ϣ

    // ��---------------------------------------------------------Serach file or dir---------------------------------------------------------------��
    void TraverseTree(TreeNode* node, string parentPath, ULONG parentclusterno, vector<vector<string>>& pathVector);// ��ȡÿ���ļ������ļ��еľ���·��
    void SearchInPathVector(const string& searchString);

    // ��---------------------------------------------------------Delete file or dir---------------------------------------------------------------��
    void DeleteSpecifiedFile(HANDLE FfileHandle, string DeleteFilePath);

    // ��---------------------------------------------------------Create file or dir---------------------------------------------------------------��
    void CreateFileOrDir(HANDLE FfileHandle, string create_file_name);

    // ��---------------------------------------------------------Copy file or dir---------------------------------------------------------------��
    void CopyFileOrDir(HANDLE FfileHandle, string Source_file_path, string destination_file_path);

    // �ͷ�Ŀ¼�ṹ
    void deleteTree(TreeNode* node) {
        if (node == nullptr) {
            return;
        }

        // �ݹ�ɾ�������ӽڵ�
        for (TreeNode* child : node->Children) {
            deleteTree(child);
        }

        // ɾ����ǰ�ڵ�
        delete node;
    }

    //FAT32��������
    ~FAT32() {
        free(mbr);
        free(dbr);
        deleteTree(Root);
    }
};
//=================================================================================================
//												END
//=================================================================================================

#endif //__SPTI_H__