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
#include <iomanip> // 用于设置输出格式
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
typedef long						LONG;  // 在一些电脑上是4Byte，在另外一些电脑上是8Byte  
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
    UCHAR BootIndicator;           // 启动标志，表示该分区是否是活动分区（0x80 表示活动，0x00 表示非活动）。
    UCHAR StartHead;               // 起始磁头号，表示分区的起始位置。
    UWORD StartSector;          // 起始扇区号和柱面号，表示分区的起始位置（柱面号和扇区号组合在一起）。
    UCHAR PartitionTypeIndicator;  // 分区类型标志，表示该分区的文件系统类型（如 0x07 表示 NTFS）。
    UCHAR EndHead;                 // 结束磁头号，表示分区的结束位置。
    UWORD EnDSector;            // 结束扇区号和柱面号，表示分区的结束位置（柱面号和扇区号组合在一起）。
    ULONG RelativeSector;          // 相对扇区号，表示从磁盘的第一个扇区到该分区起始扇区的偏移量（占用 4 字节）。
    ULONG SectorsInPartition;   // 分区中扇区的总数，用于表示分区的大小（占用 4 字节）。
};

struct MBR
{
    Partition Partitions[1];        // 表示当前有1个分区
};

struct BPB
{
    UWORD BytesPersector;            // 每扇区的字节数，通常为512字节或1024字节等。
    UCHAR SectorsPerCluster;         // 每个簇的扇区数，一个簇由多个扇区组成，是文件系统的最小分配单位。
    UWORD ReservedSectors;           // 保留扇区数，通常指从卷的起始位置到第一个FAT表之间的扇区数。
    UCHAR NumberOfFATs;              // FAT表的个数，通常为2个，用于冗余保护。
    UWORD RootEntries;               // 根目录区的条目数，表示根目录中最多能容纳的文件和子目录数量。
    UWORD SectorsOnSmallVolumes;     // 卷上扇区总数，如果小于65535则使用这个字段，否则使用SectorsOnLargeVolumes字段。
    UCHAR MediaDescriptor;           // 介质描述符，用于标识介质类型，例如硬盘、软盘等。
    UWORD SectorsPerFATOnSmallVolumes; // 每个FAT表占用的扇区数（针对小卷，即FAT12或FAT16）。
    UWORD SectorsPerTrack;           // 每磁道的扇区数，通常用于CHS寻址模式。
    UWORD Heads;                     // 磁头数，通常用于CHS寻址模式。
    ULONG HiddenSectors;             // 隐藏扇区数，从卷的起始位置到实际数据区域的偏移量（适用于非启动分区）。
    ULONG SectorsOnLargeVolumes;     // 卷上总扇区数（针对大卷，即FAT32或更大）。

    ULONG SectorsPerFAT;             // 每个FAT占用的扇区数（针对大卷，即FAT32）。
    UWORD ExtendedFlags;             // 拓展标志，通常用于FAT32中的镜像操作或卷标记。
    UWORD Version;                   // 文件系统版本号，通常是FAT32的版本信息。
    ULONG RootDir1stCluster;         // 根目录的起始簇号，表示根目录在数据区中的位置（适用于FAT32）。
    UWORD FSInfoSector;              // 文件系统信息扇区号，表示文件系统信息区的相对扇区号。
    UWORD BackupBootSector;          // BootSector的备份扇区号，表示备份启动扇区的相对扇区号。

    ULONG size;
};


struct DBR
{
    ULONG JMP;                      // JMP instruction(跳转指令,将程序执行流程跳转到引导程序处) 
    ULONGLONG OME;                  // OEM, 其内容由创建该文件系统的OEM厂商具体安排
    BPB bpb;
};

struct File {
    vector<UCHAR> FileName = {};        // 文件名
    wstring FileNameString = L"";        // 字符串类型的文件名

    vector<UCHAR> Extension = {};       // 扩展名 (3字节)
    wstring ExtensionString = L"";       // 字符串类型的扩展名

    UCHAR Attribution = 0;              // 文件属性 (1字节)
    UCHAR Reserved = 0;                 // 保留位 (1字节)
    UCHAR CR = 0;                       // 创建时间的秒数 (1字节, 精确到2秒)

    vector<UCHAR> CreationTime = {};   // 创建时间 (2字节, HHMM格式)
    vector<UCHAR> CreationDate = {};   // 创建日期 (2字节, YYYYMMDD格式)
    string CreationTimeString = "";   // 字符串类型的创建时间

    vector<UCHAR> AccessDate = {};     // 最后访问日期 (2字节, YYYYMMDD格式)
    string AccessDateString = "";   // 字符串类型的创建时间
    ULONG ClusterNo = 0;                // 开始簇号 (4字节)

    vector<UCHAR> ModifyTime = {};     // 最后修改时间 (2字节, HHMM格式)
    vector<UCHAR> ModifyDate = {};     // 最后修改日期 (2字节, YYYYMMDD格式)
    string ModifyTimeString = "";     // 字符串类型的修改时间

    vector<UCHAR> Size = {};            // 文件大小 (4字节)
    string SizeString = "";            // 字符串类型的文件大小

    // 定义重置函数，清空所有字段
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

// 使用树结构来保存文件目录结构
struct TreeNode {
    File FileInfo;
    vector<TreeNode*> Children;
    TreeNode* ParentNode = NULL; //父节点
    bool IsFolderFlag; //标记当前节点是否为文件夹。1：文件夹，0:文件

    TreeNode(bool isFolder) : IsFolderFlag(isFolder) {} // 构造函数，初始化节点
};

// FAT32
class FAT32 {
public:
    // FAT32中的结构
    MBR* mbr = new MBR();             // 主引导记录（Master Boot Record）
    DBR* dbr = new DBR();             // DOS引导记录（Disk Boot Record），也称为卷引导记录
    TreeNode* Root = new TreeNode(1);       // 根目录节点，表示文件系统的根目录
       // 文件句柄
    vector<vector<string>> FileAbsolutePath; //每个文件或者文件夹的绝对路径

    // FAT32中各个部分的起始地址
    ULONG mbrStartAddr;  // MBR的起始地址
    ULONG dbrStartAddr;  // DBR的起始地址
    ULONG FATStartAddr1;  // FAT表的起始地址
    ULONG FATStartAddr2;  // FAT表2的起始地址
    ULONG DataRegionAddr; // 数据区的起始地址

    //FAT32构造函数
    FAT32(HANDLE FAT32FileHandle, wstring Name) {
        // 初始化根节点
        Root->FileInfo.FileNameString = Name;
        Root->FileInfo.ClusterNo = 2;

        // 初始化mbr开始地址
        mbrStartAddr = 0;

        // 解析MBR
        ParsingMBR(FAT32FileHandle);

        // 初始化dbr开始地址
        dbrStartAddr = mbr->Partitions[0].RelativeSector;

        // 解析DBR
        ParsingDBR(FAT32FileHandle);

        //获取FAT表1和FAT表2的开始地址
        FATStartAddr1 = dbrStartAddr + dbr->bpb.ReservedSectors;
        FATStartAddr2 = FATStartAddr1 + dbr->bpb.SectorsPerFAT;
        

        // 获取DATA区域的开始地址
        DataRegionAddr = dbrStartAddr + dbr->bpb.ReservedSectors + (dbr->bpb.SectorsPerFAT << 1);

        //解析整个目录结构
        CreateCataloge(FAT32FileHandle);

        ////获取每个文件或者文件夹的绝对路径
        //TraverseTree(Root, Root->FileInfo.FileNameString, Root->FileInfo.ClusterNo, FileAbsolutePath);

        // 打印整个目录结构
        cout << "<------------------------------------------目录结构------------------------------------------->" << endl;
        cout << "Create Times       " << '\t' << "Last Change Times   " << '\t' << "Access Times   " << '\t' << "Type          " << '\t' << "File Size"<< '\t' << "File Name" << endl;
        PrintDirectory(Root, 0);
        printf("\n");
    }

    // FAT32中的成员函数
    void ParsingMBR(HANDLE FfileHandle);
    void ParsingDBR(HANDLE FfileHandle);
    void ParsingDir(HANDLE FfileHandle, ULONG DataBaseAddress, TreeNode* Node);

    void CreateCataloge(HANDLE FfileHandle); // 使用树结构来保存整个目录
    void PrintDirectory(TreeNode* Node, int level); //打印整个目录结构

    TreeNode* FindNode(string FilePath); //寻找某个文件或者文件夹所在节点
    ULONG DeleteFileOrDirInCluster(HANDLE FfileHandle, string DeleteFileName, TreeNode* ParentNode);//删除单个文件或者目录
    void DeleteFileClusterFromFAT(HANDLE FfileHandle, vector<ULONG> Clusters);//删除FAT表中对应的簇

    ULONG FAT32::FindFreeCluster(HANDLE FfileHandle);//在FAT表中寻找空闲的簇
    vector<ULONG> FindFileClusterFromFAT(HANDLE FfileHandle, ULONG clusternos);//在FAT表中寻找文件的所有簇
    vector<ULONG> FindFreeClusterFromFATForCopy(HANDLE FfileHandle, ULONG ClusterNumber);// 在FAT中找到能够存放文件的簇并修改标志
    vector<UCHAR> FindFileOrDirCode(HANDLE FfileHandle, string Name, TreeNode* parentNode); // 找到文件或者文件夹对应的信息以及在父目录下的编码
    void CopySourceCodeToDestination(HANDLE FfileHandle, string FileName, TreeNode* DestinationNode, vector<UCHAR> SourceCode, ULONG SatrtCluster);// 复制源文件Code到目标路径下
    void CopySourceFileContentToDestination(HANDLE FfileHandle, vector<ULONG> SourceClusters, vector<ULONG> DestinationClusters); // 复制源文件内容到目标路径下
    void RecoverFile(HANDLE FfileHandle, string file_path);//恢复删除的图片
    
    File FindFileNameForRecover(HANDLE FfileHandle, TreeNode* parentnode, string filename);// 在父目录下寻找文件名
    void ModifyClusterInfo(HANDLE FfileHandle, ULONG number, ULONG next_cluster,int is_add_flag); // 修改分区的信息

    // 《---------------------------------------------------------Serach file or dir---------------------------------------------------------------》
    void TraverseTree(TreeNode* node, string parentPath, ULONG parentclusterno, vector<vector<string>>& pathVector);// 获取每个文件或者文件夹的绝对路径
    void SearchInPathVector(const string& searchString);

    // 《---------------------------------------------------------Delete file or dir---------------------------------------------------------------》
    void DeleteSpecifiedFile(HANDLE FfileHandle, string DeleteFilePath);

    // 《---------------------------------------------------------Create file or dir---------------------------------------------------------------》
    void CreateFileOrDir(HANDLE FfileHandle, string create_file_name);

    // 《---------------------------------------------------------Copy file or dir---------------------------------------------------------------》
    void CopyFileOrDir(HANDLE FfileHandle, string Source_file_path, string destination_file_path);

    // 释放目录结构
    void deleteTree(TreeNode* node) {
        if (node == nullptr) {
            return;
        }

        // 递归删除所有子节点
        for (TreeNode* child : node->Children) {
            deleteTree(child);
        }

        // 删除当前节点
        delete node;
    }

    //FAT32析构函数
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