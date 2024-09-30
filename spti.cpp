/**************************************************************************************************/
/*																							      */
/*					Copyright (c) 2018-2024 by Hosin Global Electronics Co., LTD.				  */
/*																								  */
/**************************************************************************************************/
#include "spti.h"

// 将vector<UCHAR> 转换为 ULONG
ULONG vectorToULong(const std::vector<unsigned char>& vec) {
    ULONG size = 0;
    if (vec.size() >= 4) {
        // 将 4 个字节合并为 ULONG（假设小端字节序）
        size = (static_cast<unsigned long>(vec[0]) |
            (static_cast<unsigned long>(vec[1]) << 8) |
            (static_cast<unsigned long>(vec[2]) << 16) |
            (static_cast<unsigned long>(vec[3]) << 24));
    }
    return size;
}

// 格式化文件大小为字符串
std::string formatFileSize(unsigned long size) {
    const unsigned long KB = 1024;
    const unsigned long MB = KB * 1024;
    const unsigned long GB = MB * 1024;

    std::ostringstream oss;

    if (size >= GB) {
        oss << std::fixed << std::setprecision(2) << static_cast<double>(size) / GB << " GB";
    }
    else if (size >= MB) {
        oss << std::fixed << std::setprecision(2) << static_cast<double>(size) / MB << " MB";
    }
    else if (size >= KB) {
        oss << std::fixed << std::setprecision(2) << static_cast<double>(size) / KB << " KB";
    }
    else {
        oss << size << " Bytes";
    }

    return oss.str();
}

// 打印读取的Sector的数据
void PrintData(UCHAR* BUFFER, string name) {
    // 打印读取的数据
    printf("====================================%s=====================================\n", name.c_str());
    for (int i = 0; i < (512); i++)
    {
        printf("%02x  ", BUFFER[i]);
        if ((i > 0) && (((i + 1) % 16) == 0))
        {
            printf("\n");
        }
    }

    printf("=========================================================================\n");
}

// 发送SCSI命令的函数
BOOL IssueCDB(HANDLE fileHandle, UCHAR ucCDBLength, PUCHAR CDBData, UCHAR ucDataIn, PVOID pDataBuf, ULONG usBufLen)
{
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    BOOL status = 0;
    ULONG length = 0, returned = 0;
    UCHAR ucCount;

    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER)); // 清空结构体

    // 设置SCSI_PASS_THROUGH_DIRECT结构体的参数
    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = ucCDBLength;
    sptdwb.sptd.DataIn = ucDataIn; // 数据传输方向，数据输入还是输出
    sptdwb.sptd.SenseInfoLength = 24; // Sense信息长度
    sptdwb.sptd.DataTransferLength = usBufLen; // 数据传输长度
    sptdwb.sptd.TimeOutValue = 9000; // 超时值
    sptdwb.sptd.DataBuffer = pDataBuf; // 数据缓冲区
    sptdwb.sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf); // Sense信息的偏移量

    // 将CDB命令数据拷贝到结构体中
    for (ucCount = 0; ucCount < 16; ucCount++)
    {
        sptdwb.sptd.Cdb[ucCount] = CDBData[ucCount];
    }

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    // 发送IOCTL命令
    status = DeviceIoControl(fileHandle,
        IOCTL_SCSI_PASS_THROUGH_DIRECT, // IOCTL命令
        &sptdwb, // 输入缓冲区
        length,
        &sptdwb, // 输出缓冲区
        length,
        &returned, // 实际返回的字节数
        FALSE); // 不使用重叠IO


    //printf("Sense Data: ");
    //for (int i = 0; i < sptdwb.sptd.SenseInfoLength; i++) {
    //    printf("%02X ", sptdwb.ucSenseBuf[i]);
    //}
    //printf("\n");
    return status;
}

// 读取数据函数
UCHAR UFD_READ(HANDLE fileHandle, ULONG ulLBA, USHORT usLen, PUCHAR pucBuffer, UCHAR ucLUN)
{
    /**
     * @brief 通过SCSI指令读取指定LBA地址上的数据块。
     *
     * @param fileHandle 文件句柄，用于访问和操作设备。
     * @param ulLBA 要读取的逻辑块地址（LBA），即数据块在存储介质上的位置。
     * @param usLen 要读取的数据块数，每个数据块通常为512字节。
     * @param pucBuffer 指向存储读取数据的缓冲区的指针。
     * @param ucLUN 逻辑单元号（LUN），用于标识多逻辑单元设备的具体单元。
     *
     * @return UCHAR 操作状态，通常返回布尔类型状态（非0表示成功，0表示失败）。
     */
    UCHAR ucCDB[16];
    BOOL status;
    ULONG Len = (ULONG)usLen * 512; // 计算数据长度

    ZeroMemory(ucCDB, 16); // 清空CDB命令
    ucCDB[0] = SCSIOP_READ; // 设置SCSI读取命令
    ucCDB[1] = ucLUN << 5; // 设置LUN
    ucCDB[2] = (UCHAR)(ulLBA >> 24); // 设置LBA高位
    ucCDB[3] = (UCHAR)(ulLBA >> 16); // 设置LBA中位
    ucCDB[4] = (UCHAR)(ulLBA >> 8); // 设置LBA低位
    ucCDB[5] = (UCHAR)(ulLBA); // 设置LBA低位
    ucCDB[7] = (UCHAR)(usLen >> 8); // 设置长度高位
    ucCDB[8] = (UCHAR)(usLen); // 设置长度低位

    // 发送SCSI命令并读取数据
    status = IssueCDB(fileHandle, CDB10GENERIC_LENGTH, ucCDB, SCSI_IOCTL_DATA_IN, pucBuffer, Len);
    if (!status)
    {
        printf("Error in ufd read\n"); // 读取失败
    }

    if (DEBUG_Flag) { cout << "read:" << status << endl; }
    return status;
}

// 写入数据函数
UCHAR UFD_WRITE(HANDLE fileHandle, ULONG ulLBA, USHORT usLen, PUCHAR pucBuffer, UCHAR ucLUN)
{
    UCHAR ucCDB[16];
    BOOL status;
    ULONG Len = (ULONG)usLen * 512; // 计算数据长度

    ZeroMemory(ucCDB, 16); // 清空CDB命令
    ucCDB[0] = SCSIOP_WRITE; // 设置SCSI写入命令
    ucCDB[1] = ucLUN << 5; // 设置LUN
    ucCDB[2] = (UCHAR)(ulLBA >> 24); // 设置LBA高位
    ucCDB[3] = (UCHAR)(ulLBA >> 16); // 设置LBA中位
    ucCDB[4] = (UCHAR)(ulLBA >> 8); // 设置LBA低位
    ucCDB[5] = (UCHAR)(ulLBA); // 设置LBA低位
    ucCDB[7] = (UCHAR)(usLen >> 8); // 设置长度高位
    ucCDB[8] = (UCHAR)(usLen); // 设置长度低位
    

    // 发送SCSI命令并写入数据
    status = IssueCDB(fileHandle, CDB10GENERIC_LENGTH, ucCDB, SCSI_IOCTL_DATA_OUT, pucBuffer, Len);

    if (DEBUG_Flag) {
        if (!status)
        {
            DWORD error = GetLastError();
            printf("Error in ufd write, Error: %lu\n", error); // 写入失败
        }
        else {
            cout << "Write Successed, status: " << status << endl;
        }

    }
    
    return status;
}

// 修改FINFO
void FAT32::ModifyClusterInfo(HANDLE FfileHandle, ULONG number, ULONG next_cluster, int is_add_flag) {
    UCHAR ubDataBuf[512] = { 0 }; // 数据缓冲区

    UFD_READ(FfileHandle, dbrStartAddr + 1, 1, ubDataBuf, 0);

    ULONG cluster_number = ubDataBuf[491] << 24 | ubDataBuf[490] << 16 | ubDataBuf[489] << 8 | ubDataBuf[488];

    if (is_add_flag) {
        cluster_number += number;
    }
    else {
        cluster_number -= number;
    }

    ubDataBuf[488] = cluster_number & 0xff;
    ubDataBuf[489] = (cluster_number >> 8) & 0xff;
    ubDataBuf[490] = (cluster_number >> 16) & 0xff;
    ubDataBuf[491] = (cluster_number >> 24) & 0xff;

    ubDataBuf[492] = next_cluster & 0xff;
    ubDataBuf[493] = (next_cluster >> 8) & 0xff;
    ubDataBuf[494] = (next_cluster >> 16) & 0xff;
    ubDataBuf[495] = (next_cluster >> 24) & 0xff;

    UFD_WRITE(FfileHandle, dbrStartAddr + 1, 1, ubDataBuf, 0);
}

//解析MBR，找到每个Partition的信息
void FAT32::ParsingMBR(HANDLE FileHandle) {
    UCHAR DataBuffer[512] = { 0 };
    UFD_READ(FileHandle, mbrStartAddr, 1, DataBuffer, 0);

    if (DEBUG_Flag){ PrintData(DataBuffer, "MBR"); }
    
    for (int i = 0; i < 4; i++) {
        int StartIndex = Partition_Table_Start_Index + i * 16;

        mbr->Partitions[i].BootIndicator = DataBuffer[StartIndex];
        mbr->Partitions[i].StartHead = DataBuffer[StartIndex + 1];
        mbr->Partitions[i].StartSector = DataBuffer[StartIndex + 3] << 8 | DataBuffer[StartIndex + 2];
        mbr->Partitions[i].PartitionTypeIndicator = DataBuffer[StartIndex + 4];
        mbr->Partitions[i].EndHead = DataBuffer[StartIndex + 5];
        mbr->Partitions[i].EnDSector = DataBuffer[StartIndex + 7] << 8 | DataBuffer[StartIndex + 6];
        mbr->Partitions[i].RelativeSector = (DataBuffer[StartIndex + 11]<< 24) | (DataBuffer[StartIndex + 10]<< 16) | (DataBuffer[StartIndex + 9] << 8) | DataBuffer[StartIndex + 8];
        mbr->Partitions[i].SectorsInPartition = (DataBuffer[StartIndex + 15] << 24) | (DataBuffer[StartIndex + 14] << 16) | (DataBuffer[StartIndex + 13] << 8) | DataBuffer[StartIndex + 12];
    }
}

//解析DBR, 获取BPB信息
void FAT32::ParsingDBR(HANDLE FileHandle) {
    UCHAR DataBuffer[512] = { 0 };
    UFD_READ(FileHandle, dbrStartAddr, 1, DataBuffer, 0);

    dbr->bpb.BytesPersector = DataBuffer[BPB_Start_Index + 1] << 8 | DataBuffer[BPB_Start_Index];
    dbr->bpb.SectorsPerCluster = DataBuffer[BPB_Start_Index + 2];
    dbr->bpb.ReservedSectors = DataBuffer[BPB_Start_Index + 4] << 8 | DataBuffer[BPB_Start_Index+3];
    dbr->bpb.NumberOfFATs = DataBuffer[BPB_Start_Index + 5];
    dbr->bpb.RootEntries = DataBuffer[BPB_Start_Index + 7] << 8 | DataBuffer[BPB_Start_Index + 6];
    dbr->bpb.SectorsOnSmallVolumes = DataBuffer[BPB_Start_Index + 9] << 8 | DataBuffer[BPB_Start_Index + 8];
    dbr->bpb.MediaDescriptor = DataBuffer[BPB_Start_Index + 10];
    dbr->bpb.SectorsPerFATOnSmallVolumes = DataBuffer[BPB_Start_Index + 12] << 8 | DataBuffer[BPB_Start_Index + 11];
    dbr->bpb.SectorsPerTrack = DataBuffer[BPB_Start_Index + 14] << 8 | DataBuffer[BPB_Start_Index + 13];
    dbr->bpb.Heads = DataBuffer[BPB_Start_Index + 16] << 8 | DataBuffer[BPB_Start_Index + 15];
    dbr->bpb.HiddenSectors = DataBuffer[BPB_Start_Index + 20] << 24 | DataBuffer[BPB_Start_Index + 19] << 16 | DataBuffer[BPB_Start_Index + 18] << 8 | DataBuffer[BPB_Start_Index + 17];
    dbr->bpb.SectorsOnLargeVolumes = DataBuffer[BPB_Start_Index + 24] << 24 | DataBuffer[BPB_Start_Index + 23] << 16 | DataBuffer[BPB_Start_Index + 22] << 8 | DataBuffer[BPB_Start_Index + 21];


    dbr->bpb.SectorsPerFAT = DataBuffer[BPB_Start_Index + 28] << 24 | DataBuffer[BPB_Start_Index + 27] << 16 | DataBuffer[BPB_Start_Index + 26] << 8 | DataBuffer[BPB_Start_Index + 25];
    dbr->bpb.ExtendedFlags = DataBuffer[BPB_Start_Index + 30] << 8 | DataBuffer[BPB_Start_Index + 29];
    dbr->bpb.Version = DataBuffer[BPB_Start_Index + 32] << 8 | DataBuffer[BPB_Start_Index + 31];
    dbr->bpb.RootDir1stCluster = DataBuffer[BPB_Start_Index + 36] << 24 | DataBuffer[BPB_Start_Index + 35] << 16 | DataBuffer[BPB_Start_Index + 34] << 8 | DataBuffer[BPB_Start_Index + 33];
    dbr->bpb.FSInfoSector = DataBuffer[BPB_Start_Index + 38] << 8 | DataBuffer[BPB_Start_Index + 37];
    dbr->bpb.BackupBootSector = DataBuffer[BPB_Start_Index + 40] << 8 | DataBuffer[BPB_Start_Index + 39];
}

// 将某个索引区域的编码拼接起来
vector<UCHAR> ParsingInfo(UCHAR *ubDataBu, int start, int length) {
    vector<UCHAR> info;

    for (int i = 0; i < length; i++) {
        info.insert(info.end(), ubDataBu[start + i]);
    }
    return info;
}

// 打印 vector<UCHAR> 中的元素
void printVectorUCHAR(const vector<UCHAR>& vec) {
    cout << "Vector<UCHAR> elements: ";
    for (UCHAR elem : vec) {
        cout << "0x" << hex << setw(2) << setfill('0') << (int)elem << " ";
    }
    cout << endl;
}

// 将16进制转为字符串类型的时间
string HexToTime(vector<UCHAR> Date, vector<UCHAR> time = {}) {
    if (Date.size() < 2) {
        // 如果 Date 为空或长度不足，返回空字符串
        return "";
    }

    int year = 1980, month = 1, day = 1;  // 默认值

    if (!Date.empty()) {
        ULONG this_date = Date[1] << 8 | Date[0];
        year = (this_date >> 9) + 1980;
        month = (this_date >> 5) & 0x0F;
        day = this_date & 0x1F;
    }

    // 如果 time 为空或长度不足，返回仅包含日期的字符串
    if (time.empty() || time.size() < 2) {
        char date_buffer[11]; // 仅日期部分的格式化缓冲区
        snprintf(date_buffer, sizeof(date_buffer), "%04d-%02d-%02d", year, month, day);
        return string(date_buffer);
    }

    int hour = 0, minute = 0, second = 0; // 默认值
    ULONG this_time = time[1] << 8 | time[0];
    hour = this_time >> 11;
    minute = (this_time >> 5) & 0x3F;
    second = (this_time & 0x1F) * 2;

    // 格式化为标准日期时间字符串
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);

    return string(buffer);
}

// 将十六进制值转换为对应的字符并生成字符串
//string hexToUtf8(const vector<UCHAR>& hexValues) {
//    string result;
//    for (UCHAR hexValue : hexValues) {
//        result += static_cast<char>(hexValue);
//    }
//    result.erase(std::remove(result.begin(), result.end(), ' '), result.end());
//    return result;
//}

wstring hexToUtf8(const vector<UCHAR>& hexData) {
    wstring result;
    for (size_t i = 0; i < hexData.size(); i += 2) {
        wchar_t wchar = (wchar_t(hexData[i + 1]) << 8) | wchar_t(hexData[i]);
        result += wchar;
    }
    result.erase(std::remove(result.begin(), result.end(), ' '), result.end());
    return result;
}


//解析目录，并建立树形结构来保存
void FAT32::ParsingDir(HANDLE pfileHandle, ULONG DataBaseAddress, TreeNode* Node) {
    UCHAR DataBuffer[512] = { 0 }; // 数据缓冲区
    int flag = 1;//是否需要读取下一个Sector
    int index; //每个文件的起始索引

    File file = File(); // 文件信息

    // 解析根目录
    while (flag) {
        UFD_READ(pfileHandle, DataBaseAddress, 1, DataBuffer, 0);
        DataBaseAddress = DataBaseAddress + 1;
        
        index = 0;
        
        while (index < 512) {
            UCHAR FileMark = DataBuffer[index];
            UCHAR LongFileNameFlag = DataBuffer[index+11];

            if (FileMark == 0xE5 || FileMark == 0x2E) { // 如果是E5代表该文件已经删除了, 2E是目录的上一级和当前目录
                index = index + 32;
                continue;
            }
            else if (FileMark == 0x00) {
                flag = 0;
                break;
            }
            else if(LongFileNameFlag == 0x0F){ //如果是0F代表此时读取的是长文件名
                vector<UCHAR> temp_1 = ParsingInfo(DataBuffer, index + 1, 10);
                vector<UCHAR> temp_2 = ParsingInfo(DataBuffer, index + 14, 12);
                vector<UCHAR> temp_3 = ParsingInfo(DataBuffer, index + 28, 4);


                std::vector<int> mergedVec;
                mergedVec.insert(mergedVec.end(), temp_1.begin(), temp_1.end());
                mergedVec.insert(mergedVec.end(), temp_2.begin(), temp_2.end());
                mergedVec.insert(mergedVec.end(), temp_3.begin(), temp_3.end());

                file.FileName.insert(file.FileName.begin(), mergedVec.begin(), mergedVec.end());
            }
            else { //此时读取的是短文件名
                if (file.FileName.empty()) { // 文件名为空说明该文件是短文件名，按照短文件名进行解析。
                    //读取文件名
                    vector<UCHAR> short_file_name = ParsingInfo(DataBuffer, index, 8);
                    file.FileName.insert(file.FileName.begin(), short_file_name.begin(), short_file_name.end());
                    file.FileNameString = hexToUtf8(file.FileName); //储存真实的文件名
                }

                file.FileNameString = hexToUtf8(file.FileName); //储存真实的文件名

                file.Extension = ParsingInfo(DataBuffer, index + 8, 3);
                file.ExtensionString = hexToUtf8(file.Extension); //储存真实的扩展名

                file.Attribution = DataBuffer[index + 11];
                file.Reserved = DataBuffer[index + 12];
                file.CR = DataBuffer[index + 13];

                file.CreationTime = ParsingInfo(DataBuffer, index + 14, 2);
                file.CreationDate = ParsingInfo(DataBuffer, index + 16, 2);
                file.CreationTimeString = HexToTime(file.CreationDate, file.CreationTime);// 文件的创建时间

                file.AccessDate = ParsingInfo(DataBuffer, index + 18, 2);
                file.AccessDateString = HexToTime(file.AccessDate);

                vector<UCHAR> hclusterno = ParsingInfo(DataBuffer, index + 20, 2);
                vector<UCHAR> lclusterno = ParsingInfo(DataBuffer, index + 26, 2);
                file.ClusterNo = hclusterno[1] << 24 | hclusterno[0] << 16 | lclusterno[1] << 8 | lclusterno[0]; // 文件起始簇号

                file.ModifyTime = ParsingInfo(DataBuffer, index + 22, 2);
                file.ModifyDate = ParsingInfo(DataBuffer, index + 24, 2);
                file.ModifyTimeString = HexToTime(file.ModifyDate, file.ModifyTime);  // 文件的修改时间

                file.Size = ParsingInfo(DataBuffer, index + 28, 4);
                file.SizeString = formatFileSize(vectorToULong(file.Size));

                // 存储到树结构中
                TreeNode* this_node = new TreeNode(1);
                if (file.Attribution != 0x10)
                {
                    this_node->IsFolderFlag = 0;
                }
                this_node->FileInfo = file;
                this_node->ParentNode = Node;
                Node->Children.push_back(this_node);

                //重置结构体等待下一个文件
                file.reset(); 
            }
            index = index + 32;
        }
    }
}

// 使用层次遍历和每个目录的起始簇号读取该目录的所有文件和目录。
void FAT32::CreateCataloge(HANDLE FileHandle) {
    if (Root == nullptr) return;

    // 创建一个队列
    queue<TreeNode*> q; 

    // 根节点入队
    q.push(Root); 
    
    while (!q.empty()) {
        TreeNode* Node = q.front(); 
        q.pop();                   // 出队一个元素

        // 寻找该目录的所有文件和目录
        ParsingDir(FileHandle, DataRegionAddr + (Node->FileInfo.ClusterNo - 2) * dbr->bpb.SectorsPerCluster, Node);

        // 将该目录下的所有目录入队
        for (auto child : Node->Children) {
            if (child->IsFolderFlag == 1) {
                q.push(child);
            }
        }
    }
}

// 打印整个目录结构
void FAT32::PrintDirectory(TreeNode* Node, int level) {
    if (Node == nullptr) return;

    wstring name;
    string type;

    // 打印当前目录的名称，前面加上适当数量的 '-'
    if (Node->IsFolderFlag == 1) {
        name = Node->FileInfo.FileNameString;
        type = "Directory";
    }
    else {
        name = Node->FileInfo.FileNameString + L"." + Node->FileInfo.ExtensionString;
        type = "Archive  ";
    }

    name.erase(remove(name.begin(), name.end(), ' '), name.end());
    
    if (level != 0) {
        cout << Node->FileInfo.CreationTimeString << '\t'
            << Node->FileInfo.ModifyTimeString << '\t'
            << Node->FileInfo.AccessDateString << '\t'
            << type << '\t'
            << Node->FileInfo.SizeString << '\t' << '\t'
            << "|" + string(level * 2, '-');
        wcout << name << endl;
    }

    // 递归打印所有子目录
    for (TreeNode* child : Node->Children) {
        PrintDirectory(child, level + 1);
    }
}

// 将字符转为小写
string ToLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// 获取每个文件或者文件夹的绝对路径
void FAT32::TraverseTree(TreeNode* Node, string ParentPath, ULONG ParentClusterNo, vector<vector<string>>& PathVector) {
    if (!Node) return;

    // 当前节点的路径
    string currentPath = ParentPath.empty() ? Node->FileInfo.FileNameString : ParentPath + "/" + Node->FileInfo.FileNameString;

    // 确定节点类型
    string nodeType = Node->IsFolderFlag ? "Directory" : "File";

    // 保存文件名、父目录、节点类型、父目录的簇号（用于删除）、当前节点的簇号（用于生成文件或者文件夹）
    if (Node->IsFolderFlag) {
        PathVector.push_back({ Node->FileInfo.FileNameString, ParentPath, nodeType });

        // 递归遍历子节点
        for (TreeNode* child : Node->Children) {
            TraverseTree(child, currentPath, Node->FileInfo.ClusterNo, PathVector);
        }
    }
    else {
        PathVector.push_back({ Node->FileInfo.FileNameString + "." + Node->FileInfo.ExtensionString, ParentPath, nodeType });
    }
}

// 搜索并输出包含指定字符串的文件或文件夹
void FAT32::SearchInPathVector(const std::string& SearchString) {
    printf("---------------------------------------------------------------------------------------------------\n");
    cout << "The Result about Searching String:" << SearchString << endl;
    printf("---------------------------------------------------------------------------------------------------\n");

    vector<vector<string>> result;
    string lowerSearchString = ToLower(SearchString);

    for (const auto& entry : FileAbsolutePath) {
        std::string name = ToLower(entry[0]);  // 文件或目录名
        std::string parentPath = ToLower(entry[1]);  // 父目录路径
        std::string nodeType = ToLower(entry[2]);  // 节点类型

        // 检查文件名或目录名是否包含搜索字符串
        if (string(name).find(lowerSearchString) != std::string::npos) {
            result.push_back(entry);  // 将匹配的条目加入结果向量
        }
    }

    if (!result.empty()) {
        for (const auto& entry : result) {
            std::cout << "Name: " + entry[0] + ", Parent Directory: " + entry[1] + ", Type: " + entry[2] << std::endl;
        }
    }
    else {
        std::cout << "No files or directories found containing \"" << SearchString << "\"." << std::endl;
    }

    printf("\n");
    printf("\n");
}

// 根据“/”分割String
vector<std::string> DevideString(string heihei){
    std::vector<std::string> parts;
    size_t pos = 0;
    std::string token;
    std::string delimiter = "/";

    while ((pos = heihei.find(delimiter)) != std::string::npos) {
        token = heihei.substr(0, pos);
        parts.push_back(token);
        heihei.erase(0, pos + delimiter.length());
    }
    parts.push_back(heihei); // Don't forget to add the last part

    return parts;
}

// 寻找某个文件或者目录的父目录
TreeNode* FAT32::FindNode(string FilePath) {
    if (Root == nullptr) return NULL;

    // 划分字符串
    vector<std::string> parts = DevideString(FilePath);

    if (parts.empty()) return NULL;

    TreeNode* Node = Root; //找到的父节点
    unsigned int index = 1;
    int flag;

    // 创建文件时，根节点下的文件
    cout << Root->FileInfo.FileNameString << " " << FilePath << endl;
    if (Root->FileInfo.FileNameString == FilePath) {
        return Root;
    }

    while (Node != NULL) {
        flag = 1;
        for (auto child : Node->Children) { //遍历当前节点的孩子节点
            string name = child->IsFolderFlag ? child->FileInfo.FileNameString : child->FileInfo.FileNameString + "." + child->FileInfo.ExtensionString;

            // 孩子节点与index对应的路径相符
            if (name == parts[index]) {
                index += 1;
                flag = 0;
                Node = child;
                break;
            }
        }

        if (flag) {//没找到文件，退出
            Node = NULL;
        }
        if (index == parts.size()) { // 路径寻找完毕，退出
            return Node;
        }
    }
    return NULL;
}

// 将删除文件的文件名标志改为E5
ULONG FAT32::DeleteFileOrDirInCluster(HANDLE FfileHandle, string DeleteFileName, TreeNode* ParentNode) {
    ULONG DataBaseAddress = DataRegionAddr + (ParentNode->FileInfo.ClusterNo - 2) * dbr->bpb.SectorsPerCluster;

    UCHAR DataBuffer[512] = { 0 }; // 数据缓冲区
    int flag = 1;//是否需要读取下一个Sector
    int index; //每个文件的起始索引

    ULONG clusterno = 0;
    std::vector<UCHAR> filename = {};
    std::vector<UCHAR> extension = {};
    string filename_string;
    string extension_string;
    string name;

    while (flag) {
        UFD_READ(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);

        index = 0;

        while (index < 512) {
            UCHAR file_flag = DataBuffer[index];
            UCHAR file_long_flag = DataBuffer[index + 11];

            if (file_flag == 0xE5 || file_flag == 0x2E) { // 如果是E5代表该文件已经删除了
                index = index + 32;
                continue;
            }
            else if (file_flag == 0x00) {
                flag = 0;
                break;
            }
            else if (file_long_flag == 0x0F) { //如果是0F代表此时读取的是长文件名
                vector<UCHAR> temp_1 = ParsingInfo(DataBuffer, index + 1, 10);
                vector<UCHAR> temp_2 = ParsingInfo(DataBuffer, index + 14, 12);
                vector<UCHAR> temp_3 = ParsingInfo(DataBuffer, index + 28, 4);


                std::vector<int> mergedVec;
                mergedVec.insert(mergedVec.end(), temp_1.begin(), temp_1.end());
                mergedVec.insert(mergedVec.end(), temp_2.begin(), temp_2.end());
                mergedVec.insert(mergedVec.end(), temp_3.begin(), temp_3.end());

                filename.insert(filename.begin(), mergedVec.begin(), mergedVec.end());
            }
            else { // 该文件的文件名到底了
                if (filename.empty()) { // 文件名为空说明该文件是短文件名，按照短文件名进行解析。
                    //读取文件名
                    vector<UCHAR> short_file_name = ParsingInfo(DataBuffer, index, 8);
                    filename.insert(filename.begin(), short_file_name.begin(), short_file_name.end());
                }

                filename_string = hexToUtf8(filename); //储存真实的文件名

                extension = ParsingInfo(DataBuffer, index + 8, 3);
                extension_string = hexToUtf8(extension); //储存真实的扩展名

                vector<UCHAR> hclusterno = ParsingInfo(DataBuffer, index + 20, 2);
                vector<UCHAR> lclusterno = ParsingInfo(DataBuffer, index + 26, 2);

                name = DataBuffer[index + 11] == 0x10 ? filename_string : filename_string + "." + extension_string;

                if (name == DeleteFileName) { // 找到对应的文件名位置了
                    clusterno = hclusterno[1] << 24 | hclusterno[0] << 16 | lclusterno[1] << 8 | lclusterno[0]; // 文件起始簇号

                    // 文件名修改标志位为0xE5
                    int xidenx = index + 11;
                    do {
                        DataBuffer[index] = 0xE5;
                        index -= 32;
                        xidenx -= 32;
                    } while (DataBuffer[xidenx] == 0x0F);

                    UFD_WRITE(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);
                    break;
                }

                filename.clear(); //清空文件名
                extension.clear(); //清空扩展名
            }
            index = index + 32;
        }

        DataBaseAddress = DataBaseAddress + 1;
    }
    return clusterno;
}

//从FAT中删除文件的ClusterNo
void FAT32::DeleteFileClusterFromFAT(HANDLE FfileHandle, vector<ULONG> Clusters) {
    UCHAR DataBuffer[512] = { 0 }; // 数据缓冲区
    ULONG next_item;
    int start_index; // 簇号的在Sector中的开始位置

    for (auto clusterno : Clusters) {
        UFD_READ(FfileHandle, FATStartAddr1 + clusterno / 128, 1, DataBuffer, 0);

        start_index = int(clusterno % 128) * 4;
        next_item = DataBuffer[start_index + 3] << 24 | DataBuffer[start_index + 2] << 16 | DataBuffer[start_index + 1] << 8 | DataBuffer[start_index];

        while (next_item != 0x0fffffff) {
            // 将该文件对应的FAT ietm置为默认值
            DataBuffer[start_index] = 0x00;
            DataBuffer[start_index + 1] = 0x00;
            DataBuffer[start_index + 2] = 0x00;
            DataBuffer[start_index + 3] = 0x00;
            UFD_WRITE(FfileHandle, FATStartAddr1 + clusterno / 128, 1, DataBuffer, 0);
            UFD_WRITE(FfileHandle, FATStartAddr2 + clusterno / 128, 1, DataBuffer, 0);

            //读取该文件的下一个ClusterNo
            UFD_READ(FfileHandle, FATStartAddr1 + next_item / 128, 1, DataBuffer, 0);
            start_index = int(next_item % 128) * 4;
            next_item = DataBuffer[start_index + 3] << 24 | DataBuffer[start_index + 2] << 16 | DataBuffer[start_index + 1] << 8 | DataBuffer[start_index];
        }

        // 最后一个簇删除
        DataBuffer[start_index] = 0x00;
        DataBuffer[start_index + 1] = 0x00;
        DataBuffer[start_index + 2] = 0x00;
        DataBuffer[start_index + 3] = 0x00;
        UFD_WRITE(FfileHandle, FATStartAddr1 + clusterno / 128, 1, DataBuffer, 0);
        UFD_WRITE(FfileHandle, FATStartAddr2 + clusterno / 128, 1, DataBuffer, 0);
    }
}

// 删除指定的文件夹或者文件
void FAT32::DeleteSpecifiedFile(HANDLE FfileHandle, string DeleteFilePath) {
    printf("---------------------------------------------------------------------------------------------------\n");
    cout << "The Result Deleting String:" << DeleteFilePath << endl;
    printf("---------------------------------------------------------------------------------------------------\n");

    //寻找该节点
    TreeNode* DeleteFileNode = FindNode(DeleteFilePath);
    string DeleteFileName = DevideString(DeleteFilePath).back(); //删除文件的文件名
    vector<ULONG> FATStartClusterNo; // 删除文件的簇号

    //判断是否能找到该文件，以及其是文件还是文件夹
    if (DeleteFileNode == NULL) {
        cout << "No find File" << endl;
    }
    else {
        cout << "Finding File Successed. Parent Dir is " << DeleteFileNode->ParentNode->FileInfo.FileNameString;

        //根据其是文件还是目录做不同的操作。Directory or File
        if (DeleteFilePath.find('.') != std::string::npos) { // 执行删除文件对应的操作
            // 删除对应的文件
            ULONG FileStartCluster = DeleteFileOrDirInCluster(FfileHandle, DevideString(DeleteFilePath).back(), DeleteFileNode->ParentNode);

            if (FileStartCluster != 0) {
                FATStartClusterNo.push_back(FileStartCluster);
            }
        }
        else {// 执行删除文件夹对应的操作
            // 首先将当前文件夹删除
            ULONG FileStartCluster = DeleteFileOrDirInCluster(FfileHandle, DevideString(DeleteFilePath).back(), DeleteFileNode->ParentNode);

            queue<TreeNode*> q; // 创建一个队列
            // 将要删除的文件夹节点入队
            q.push(DeleteFileNode);

            while (!q.empty()) {
                TreeNode* Node = q.front();
                q.pop();                   // 出队一个元素

                for (auto child : Node->Children) {
                    string ThisDeleteFileName = child->IsFolderFlag ? child->FileInfo.FileNameString : child->FileInfo.FileNameString + "." + child->FileInfo.ExtensionString;
                    ULONG FileStartCluster = DeleteFileOrDirInCluster(FfileHandle, ThisDeleteFileName, Node);

                    if (child->IsFolderFlag == 0 && FileStartCluster != 0) { // 删除文件，并将其开始簇加入FATstartcluster中
                        FATStartClusterNo.push_back(FileStartCluster);
                    }
                    else {
                        q.push(child);
                    }
                }
            }
        }

         //删除FAT表中对应的簇
         if (!FATStartClusterNo.empty()) {
             DeleteFileClusterFromFAT(FfileHandle, FATStartClusterNo);
         }

         // 修改分区信息
         ULONG minIt = 0xFFFFFFFF;
         for (auto i : FATStartClusterNo) {
             if (i < minIt) {
                 minIt = i;
             }
         }

         ModifyClusterInfo(FfileHandle, FATStartClusterNo.size(), minIt, 1);

         cout << "Delete Successed" << endl;
    }
}

// 获取当前时间并将其转换为使用“-”连接的字符串
string getCurrentTimeString() {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为time_t类型，以获取秒级时间戳
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // 定义一个 tm 结构来存储本地时间
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);  // 使用 localtime_s 替换 localtime

    // 使用ostringstream将时间格式化为字符串，仅包含时间部分
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%H-%M-%S");  // 仅保留时间部分，并用“-”连接

    std::cout << oss.str() << std::endl;
    return oss.str();
}

// 将时间转换为 FAT 时间格式
uint16_t getFatTime(struct tm* tstruct) {
    return (tstruct->tm_hour << 11) | (tstruct->tm_min << 5) | (tstruct->tm_sec / 2);
}

// 将日期转换为 FAT 日期格式
uint16_t getFatDate(struct tm* tstruct) {
    return ((tstruct->tm_year - 80) << 9) | ((tstruct->tm_mon + 1) << 5) | tstruct->tm_mday;
}

// 反转字节顺序以实现小端存储
uint16_t toLittleEndian(uint16_t value) {
    return (value >> 8) | (value << 8);
}

// 获取当前时间，并根据相关信息创建需要保存到Data区域的信息
vector<UCHAR> CreateFileName(ULONG FileCluster) {
    vector<UCHAR> FileName(32, 0);

    // 使用当前时间作为文件的文件名称
    string timeStr = getCurrentTimeString();
    replace(timeStr.begin(), timeStr.end(), '-', '_'); // 替换'-'为'_'

    string Name = timeStr.substr(0, 8); // 取前8个字符作为文件名
    string Ext = "TXT"; // 设置扩展名为 txt

    // 填充文件名和扩展名到SFN结构中
    for (string::size_type i = 0; i < 8; ++i) {
        if (i < Name.size()) {
            FileName[i] = Name[i];
        } else {
            FileName[i] = 0x20; // 用空格填充
        }
    }
    for (string::size_type i = 0; i < 3; ++i) {
        if (i < Ext.size()) {
            FileName[8 + i] = Ext[i];
        } else {
            FileName[8 + i] = 0x20; // 用空格填充
        }
    }

    FileName[11] = 0x20; // 文件属性

    // 获取当前文件的创建时间和修改时间
    time_t now = time(0);
    struct tm tstruct;
    localtime_s(&tstruct, &now);

    uint16_t fatDate = toLittleEndian(getFatDate(&tstruct));
    uint16_t fatTime = toLittleEndian(getFatTime(&tstruct));

    FileName[14] = (fatTime >> 8) & 0xFF;
    FileName[15] = fatTime & 0xFF;
    FileName[16] = (fatDate >> 8) & 0xFF;
    FileName[17] = fatDate & 0xFF;

    FileName[22] = FileName[14];
    FileName[23] = FileName[15];
    FileName[24] = FileName[16];
    FileName[25] = FileName[17];

    return FileName;
}

// 在FAT表中寻找能够放下整个文件空闲的簇
ULONG FAT32::FindFreeCluster(HANDLE FfileHandle) {
    ULONG FreeCluster = 2;
    UCHAR ubDataBuf[512] = { 0 };
    ULONG Temp;
    unsigned int offset;


    // 读取FAT的信息
    ULONG FATBaseAddress = FATStartAddr1;
    UFD_READ(FfileHandle, FATBaseAddress, 1, ubDataBuf, 0);

    while (1) {
        offset = FreeCluster % 128;
        Temp = ubDataBuf[offset * 4 + 3] << 24 | ubDataBuf[offset * 4 + 2] << 16 | ubDataBuf[offset * 4 + 1] << 8 | ubDataBuf[offset * 4];

        if (Temp == 0x00000000) {
            ubDataBuf[offset * 4 + 3] = 0x0F;
            ubDataBuf[offset * 4 + 2] = 0xFF;
            ubDataBuf[offset * 4 + 1] = 0xFF;
            ubDataBuf[offset * 4] = 0xFF;
            UFD_WRITE(FfileHandle, FATBaseAddress, 1, ubDataBuf, 0);
            break;
        }
        else if (offset * 4 == 508) {
            FATBaseAddress += 1;
            UFD_READ(FfileHandle, FATBaseAddress, 1, ubDataBuf, 0);
        }
        else {
            FreeCluster += 1;
        }
    }

    return FreeCluster;
}

// 创建文件或文件夹
void FAT32::CreateFileOrDir(HANDLE FfileHandle, string CreateFilePath) {
    printf("---------------------------------------------------------------------------------------------------\n");
    cout << "The Result Creating String:" << CreateFilePath << endl;
    printf("---------------------------------------------------------------------------------------------------\n");

    UCHAR DataBuffer[512] = { 0 }; // 数据缓冲区
    unsigned int start_index =0;
    ULONG DataBaseAddress ;

    // 寻找该节点
    TreeNode* CreateFileNode = FindNode(CreateFilePath);
    if (CreateFileNode == NULL) {
        cout << "No find File" << endl;
    }
    else {
        // 根据文件信息在其父目录下创建文件名
       DataBaseAddress = DataRegionAddr + (CreateFileNode->FileInfo.ClusterNo - 2) * dbr->bpb.SectorsPerCluster;

        UFD_READ(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);

        while (1) {
            if (DataBuffer[start_index] == 0x00) {
                break;
            }
            else if (start_index == 480) {
                DataBaseAddress += 1;
                UFD_READ(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);
                start_index = 0;
            }
            else {
                start_index += 32;
            }
        }

        // 创建短文件名的16进制格式
        vector<UCHAR> sfnPtr = CreateFileName(0x00);

        // 放入对应位置并写入数据区域
        for (int i = 0; i < 32; i++) {
            DataBuffer[start_index + i] = sfnPtr[i];
        }

        //写回数据区域
        UFD_WRITE(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);

        cout << "Create Successed" << endl;
    }
}

// 在FAT中找到文件所有的簇
vector<ULONG> FAT32::FindFileClusterFromFAT(HANDLE FfileHandle, ULONG StartClusterNo) {
    UCHAR DataBuffer[512] = { 0 }; // 数据缓冲区
    ULONG next_item;
    int start_index; // 簇号的在Sector中的开始位置
    vector<ULONG> Clusters;
    Clusters.push_back(StartClusterNo);

    UFD_READ(FfileHandle, FATStartAddr1 + StartClusterNo / 128, 1, DataBuffer, 0);
    start_index = int(StartClusterNo % 128) * 4;
    next_item = DataBuffer[start_index + 3] << 24 | DataBuffer[start_index + 2] << 16 | DataBuffer[start_index + 1] << 8 | DataBuffer[start_index];

    while (next_item != 0x0fffffff) {
        Clusters.push_back(next_item);

        //读取该文件的下一个ClusterNo
        UFD_READ(FfileHandle, FATStartAddr1 + next_item / 128, 1, DataBuffer, 0);
        start_index = int(next_item % 128) * 4;
        next_item = DataBuffer[start_index + 3] << 24 | DataBuffer[start_index + 2] << 16 | DataBuffer[start_index + 1] << 8 | DataBuffer[start_index];
    }

    return Clusters;
}

// 找到文件或者文件夹对应的信息以及在父目录下的编码
vector<UCHAR> FAT32::FindFileOrDirCode(HANDLE FfileHandle, string FileName, TreeNode* ParentNode) {
    ULONG DataBaseAddress = DataRegionAddr + (ParentNode->FileInfo.ClusterNo - 2) * dbr->bpb.SectorsPerCluster;
    UCHAR DataBuffer[512] = { 0 }; // 数据缓冲区
    int flag = 1;//是否需要读取下一个Sector
    int index; //每个文件的起始索引

    std::vector<UCHAR> filename = {};
    std::vector<UCHAR> extension = {};
    string filename_string;
    string extension_string;
    string name;

    vector<UCHAR> Buf32;
    

    while (flag) {
        UFD_READ(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);

        index = 0;

        while (index < 512) {
            UCHAR file_flag = DataBuffer[index];
            UCHAR file_long_flag = DataBuffer[index + 11];

            if (file_flag == 0xE5 || file_flag == 0x2E) { // 如果是E5代表该文件已经删除了
                index = index + 32;
                continue;
            }
            else if (file_flag == 0x00) {
                flag = 0;
                break;
            }
            else if (file_long_flag == 0x0F) { //如果是0F代表此时读取的是长文件名
                vector<UCHAR> all32 = ParsingInfo(DataBuffer, index, 32);
                Buf32.insert(Buf32.begin(), all32.begin(), all32.end());

                vector<UCHAR> temp_1 = ParsingInfo(DataBuffer, index + 1, 10);
                vector<UCHAR> temp_2 = ParsingInfo(DataBuffer, index + 14, 12);
                vector<UCHAR> temp_3 = ParsingInfo(DataBuffer, index + 28, 4);


                std::vector<int> mergedVec;
                mergedVec.insert(mergedVec.end(), temp_1.begin(), temp_1.end());
                mergedVec.insert(mergedVec.end(), temp_2.begin(), temp_2.end());
                mergedVec.insert(mergedVec.end(), temp_3.begin(), temp_3.end());

                filename.insert(filename.begin(), mergedVec.begin(), mergedVec.end());
            }
            else { // 该文件的文件名到底了
                if (filename.empty()) { // 文件名为空说明该文件是短文件名，按照短文件名进行解析。

                    //读取文件名
                    vector<UCHAR> short_file_name = ParsingInfo(DataBuffer, index, 8);
                    filename.insert(filename.begin(), short_file_name.begin(), short_file_name.end());
                }

                vector<UCHAR> all32 = ParsingInfo(DataBuffer, index, 32);
                Buf32.insert(Buf32.begin(), all32.begin(), all32.end());

                filename_string = hexToUtf8(filename); //储存真实的文件名
                extension = ParsingInfo(DataBuffer, index + 8, 3);
                extension_string = hexToUtf8(extension); //储存真实的扩展名
            
                
                name = DataBuffer[index + 11] == 0x10 ? filename_string : filename_string + "." + extension_string;
                if (name == FileName) { // 找到对应的文件名位置了
                    flag = 0;
                    break;
                }
                filename.clear(); //清空文件名
                extension.clear(); //清空扩展名
                Buf32.clear(); //清空
            }
            index = index + 32;
        }

        DataBaseAddress = DataBaseAddress + 1;
    }
    return Buf32;
}

// 在FAT中找到能够存放文件的簇并修改标志
vector<ULONG> FAT32::FindFreeClusterFromFATForCopy(HANDLE FfileHandle, ULONG ClusterNumber) {
    UCHAR DataBuffer[512] = { 0 }; // 数据缓冲区
    vector<ULONG> Clusters;
    ULONG number=0;
    ULONG Addr = FATStartAddr1;

    // 寻找空闲的簇
    while (number < ClusterNumber) {
        UFD_READ(FfileHandle, Addr, 1, DataBuffer, 0);

        ULONG index = 0x00;

        while (index < 128) {
            ULONG next_item = DataBuffer[(index * 4) + 3] << 24 | DataBuffer[(index * 4) + 2] << 16 | DataBuffer[(index * 4) + 1] << 8 | DataBuffer[(index * 4)];
            if (next_item == 0x00) {
                Clusters.push_back(index);
                number += 1;

                if (number == ClusterNumber) {
                    break;
                }
            }
            index += 1;
        }
        
        if (index == 128) {
            Addr += 1;
        }
    }

    // 修改其标志
    for (size_t i = 0; i < Clusters.size();i++) {
        UFD_READ(FfileHandle, FATStartAddr1 + Clusters[i] / 128, 1, DataBuffer, 0);

        int start_index = int(Clusters[i] % 128) * 4;
        if (i != Clusters.size() - 1) {
            DataBuffer[start_index] = Clusters[i+1] & 0xff;
            DataBuffer[start_index+1] = Clusters[i+1] >> 8 & 0xff;
            DataBuffer[start_index+2] = Clusters[i+1] >> 16  & 0xff;
            DataBuffer[start_index+3] = Clusters[i+1] >> 24 & 0xff;
            printf("%x ,%x ,%x ,%x \n", DataBuffer[start_index], DataBuffer[start_index + 1], DataBuffer[start_index + 2], DataBuffer[start_index + 3]);
            
        }
        else {
            DataBuffer[start_index] = 0xff;
            DataBuffer[start_index + 1] = 0xff;
            DataBuffer[start_index + 2] = 0xff;
            DataBuffer[start_index + 3] = 0x0f;
        }
        
        UFD_WRITE(FfileHandle, FATStartAddr1 + Clusters[i] / 128, 1, DataBuffer, 0);
        UFD_WRITE(FfileHandle, FATStartAddr2 + Clusters[i] / 128, 1, DataBuffer, 0);
    }

    return Clusters;
}

// 复制源文件Code到目标路径下
void FAT32::CopySourceCodeToDestination(HANDLE FfileHandle, string FileName, TreeNode* DestinationNode, vector<UCHAR> SourceCode, ULONG SatrtCluster) {
    ULONG DataBaseAddress = DataRegionAddr + (DestinationNode->FileInfo.ClusterNo - 2) * dbr->bpb.SectorsPerCluster;
    UCHAR DataBuffer[512] = { 0 }; // 数据缓冲区
    int index; //每个文件的起始索引

    // 判断源文件的文件名长度(短文件名 /长文件名)
    size_t SourceCodelength = SourceCode.size() / 32;

    // 修改文件名中的开始簇号
    if (SourceCodelength == 1) {
        SourceCode[20] = (SatrtCluster >> 16) & 0xFF;
        SourceCode[21] = (SatrtCluster >> 24) & 0xFF;

        SourceCode[26] = SatrtCluster & 0xFF;
        SourceCode[27] = (SatrtCluster >> 8) & 0xFF;
    }
    else {
        SourceCode[(SourceCodelength - 1) * 32 + 20] = (SatrtCluster >> 16) & 0xFF;
        SourceCode[(SourceCodelength - 1) * 32 + 21] = (SatrtCluster >> 24) & 0xFF;

        SourceCode[(SourceCodelength - 1) * 32 + 26] = SatrtCluster & 0xFF;
        SourceCode[(SourceCodelength - 1) * 32 + 27] = (SatrtCluster >> 8) & 0xFF;
    }

    // 在目标路径下寻找合适的位置放进去
    ULONG free_number = 0;
    ULONG lastindex = 0;
    while (free_number< SourceCodelength) {
        UFD_READ(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);

        index = 0;
        while (index < 512) {
            UCHAR file_flag = DataBuffer[index];

            if (free_number == SourceCodelength) {
                lastindex = index;
                break;
            }

            if (file_flag == 0xE5 || file_flag == 0x00) {
                free_number += 1;
            }
            else {
                free_number = 0;
            }
            index += 32;
        }
        DataBaseAddress = DataBaseAddress + 1;
    }

    // 放入指定位置
    for (size_t i = 0; i < SourceCode.size(); i++) {
        DataBuffer[lastindex - (SourceCodelength) * 32 + i] = SourceCode[i];
    }

    UFD_WRITE(FfileHandle, DataBaseAddress - 1, 1, DataBuffer, 0);
}

// 复制源文件内容到目的文件中
void FAT32::CopySourceFileContentToDestination(HANDLE FfileHandle, vector<ULONG> SourceClusters, vector<ULONG> DestinationClusters) {
    ULONG SourceAddress = DataRegionAddr;
    ULONG DestinationAddress = DataRegionAddr;
    UCHAR pubDataBuf[512] = { 0 }; // 数据缓冲区

    for (size_t i = 0; i < SourceClusters.size(); i++) {
        for (ULONG j = 0; j < dbr->bpb.SectorsPerCluster; j++)
        {
            SourceAddress = SourceAddress + (SourceClusters[i]-2) * dbr->bpb.SectorsPerCluster + j;
            DestinationAddress = DestinationAddress + (DestinationClusters[i]-2) * dbr->bpb.SectorsPerCluster + j;
            printf("SourceAddress: %x, DestinationAddress: %x\n", SourceAddress, DestinationAddress);

            // 读出源数据
            UFD_READ(FfileHandle, SourceAddress, 1, pubDataBuf, 0);

            //写入目标文件
            UFD_WRITE(FfileHandle, DestinationAddress, 1, pubDataBuf, 0);
        }
    }
}

// 复制文件或者文件夹到另外一个目录下
void FAT32::CopyFileOrDir(HANDLE FfileHandle, string SourceFilePath, string DestinationFilePath) {
    //判断路径是否存在
    TreeNode* SourceNode = FindNode(SourceFilePath);
    TreeNode* DestinationNode = FindNode(DestinationFilePath);

    if (DestinationNode != NULL && SourceNode !=NULL) {//路径存在
        // 判断目的路径下是否存在同名文件或者文件夹
        vector<string> file_name = DevideString(SourceFilePath);

        // 判断目标路径下是否存在同名文件或者文件夹
        int flag = 1;
        for (auto child: DestinationNode->Children) {
            string name = child->IsFolderFlag ? child->FileInfo.FileNameString : child->FileInfo.FileNameString + "." + child->FileInfo.ExtensionString;

            if (file_name.back() == name) {
                flag = 0;
                break;
            }
        }
        if (flag) { //目标路径下不存在同名文件或者文件夹
            if (file_name.back().find(".") == std::string::npos) {//目录

            }
            else {//文件
                // 在FAT表中找到源文件所有的簇
                vector<ULONG> SourceClusters = FindFileClusterFromFAT(FfileHandle, SourceNode->FileInfo.ClusterNo);

                // 读出源文件在其父目录下的文件名
                vector<UCHAR> Sourcefilecode = FindFileOrDirCode(FfileHandle, file_name.back(), SourceNode->ParentNode);

                // 在FAT中找到能够存放文件的簇并修改标志
                vector<ULONG> DestinationClusters = FindFreeClusterFromFATForCopy(FfileHandle, SourceClusters.size());

                // 复制文件Code到目标路径下
                CopySourceCodeToDestination(FfileHandle, file_name.back(), DestinationNode, Sourcefilecode, DestinationClusters[0]);

                // 复制文件内容到目标路径下
                CopySourceFileContentToDestination(FfileHandle, SourceClusters, DestinationClusters);

                // 修改分区信息
                ModifyClusterInfo(FfileHandle, DestinationClusters.size(), DestinationClusters.back() + 1, 0);
            }
        }
        else {
            cout << "There is a file or folder with same name in Destination file path" << endl;
        }
    }
    else {//路径不存在
        if (DestinationNode == NULL) {
            cout << "Destination file path does not exist" << endl; 
        }

        if (SourceNode == NULL) {
            cout << "Source file path does not exist" << endl;
        }
        
    }
}

//获取文件或文件夹的父目录的绝对路径
string getLeadingPathComponent(const std::string& path) {
    // 找到最后一个路径分隔符的位置
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        // 如果没有找到路径分隔符，返回一个空字符串（表示没有前导路径）
        return "";
    }
    else {
        // 返回最后一个分隔符之前的部分
        return path.substr(0, pos);
    }
}

//utf转为16进制
vector<UCHAR> Utf8Hex(const std::string& utf8String) {
    vector<UCHAR> hex; 
    for (unsigned char c : utf8String) {

        hex.push_back(static_cast<int>(c));
    }
    return hex;
}

// 在父目录下寻找文件名
File FAT32::FindFileNameForRecover(HANDLE FfileHandle, TreeNode* ParentNode, string filename) {
    ULONG DataBaseAddress = DataRegionAddr + (ParentNode->FileInfo.ClusterNo - 2)*dbr->bpb.SectorsPerCluster;
    UCHAR DataBuffer[512] = { 0 }; // 数据缓冲区
    int flag = 1;//是否需要读取下一个Sector
    int index; //每个文件的起始索引

    File file = File(); // 文件信息
    string frontname = "";
    string endname = "";

    for (size_t i = 0; i < filename.size() - 4; i++) {
        frontname += filename[i];
    }

    for (size_t i = filename.size() - 3; i < filename.size(); i++) {
        endname += filename[i];
    }

    cout << frontname << " " << endname << endl;

    // 解析根目录
    while (flag) {
        UFD_READ(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);
        DataBaseAddress = DataBaseAddress + 1;

        index = 0;

        while (index < 512) {
            UCHAR file_flag = DataBuffer[index];
            UCHAR file_long_flag = DataBuffer[index + 11];

            if (file_flag == 0x2E) {
                index = index + 32;
                continue;
            }
            else if (file_flag == 0x00) {
                flag = 0;
                break;
            }
            else if (file_long_flag == 0x0F) { //如果是0F代表此时读取的是长文件名
                vector<UCHAR> temp_1 = ParsingInfo(DataBuffer, index + 1, 10);
                vector<UCHAR> temp_2 = ParsingInfo(DataBuffer, index + 14, 12);
                vector<UCHAR> temp_3 = ParsingInfo(DataBuffer, index + 28, 4);


                std::vector<UCHAR> mergedVec;
                mergedVec.insert(mergedVec.end(), temp_1.begin(), temp_1.end());
                mergedVec.insert(mergedVec.end(), temp_2.begin(), temp_2.end());
                mergedVec.insert(mergedVec.end(), temp_3.begin(), temp_3.end());

                file.FileName.insert(file.FileName.begin(), mergedVec.begin(), mergedVec.end());
            }
            else { //此时读取的是短文件名
                if (file.FileName.empty()) { // 文件名为空说明该文件是短文件名，按照短文件名进行解析。
                    //读取文件名
                    vector<UCHAR> short_file_name = ParsingInfo(DataBuffer, index, 8);
                    file.FileName.insert(file.FileName.begin(), short_file_name.begin(), short_file_name.end());
                    file.FileNameString = hexToUtf8(file.FileName); //储存真实的文件名
                }

                file.FileNameString = hexToUtf8(file.FileName); //储存真实的文件名

                file.Extension = ParsingInfo(DataBuffer, index + 8, 3);
                file.ExtensionString = hexToUtf8(file.Extension); //储存真实的扩展名

                vector<UCHAR> hclusterno = ParsingInfo(DataBuffer, index + 20, 2);
                vector<UCHAR> lclusterno = ParsingInfo(DataBuffer, index + 26, 2);
                file.ClusterNo = hclusterno[1] << 24 | hclusterno[0] << 16 | lclusterno[1] << 8 | lclusterno[0]; // 文件起始簇号

                file.Size = ParsingInfo(DataBuffer, index + 28, 4);
                file.SizeString = formatFileSize(vectorToULong(file.Size));

                if (file.ExtensionString == endname) {
                    PrintData(DataBuffer, "haha");
                    DataBuffer[index] = 0x31;
                    PrintData(DataBuffer, "haha");
                    UFD_WRITE(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);
                    UFD_READ(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);
                    PrintData(DataBuffer, "haha");
                    return file;
                }

                //重置结构体等待下一个文件
                file.reset();
            }
            index = index + 32;
        }
    }
    File file1 = File(); // 文件信息
    return file1;
}

//恢复删除的图片
void FAT32::RecoverFile(HANDLE FfileHandle, string FilePath) {
    // 寻找父目录
    cout << getLeadingPathComponent(FilePath) << endl;
    TreeNode* ParentNode = FindNode(getLeadingPathComponent(FilePath));

    // 比对文件名
    string name = DevideString(FilePath).back();
    cout << name << endl;

    // 读取开始簇和文件大小
    File HAHA = FindFileNameForRecover(FfileHandle, ParentNode, name);

    if (HAHA.FileNameString == "") {
        cout << "no file" << endl;
    }
    else {
        cout << "Recover file" << endl;

        // 去FAT 表里顺序退出该文件所占的簇
        UCHAR ubDataBuf[512] = { 0 };
        UFD_READ(FfileHandle, FATStartAddr1 + HAHA.ClusterNo / 128, 1, ubDataBuf, 0);

        ULONG start_index = (HAHA.ClusterNo % 128) * 4;
        ubDataBuf[start_index] = 0xff;
        ubDataBuf[start_index + 1] = 0xff;
        ubDataBuf[start_index + 2] = 0xff;
        ubDataBuf[start_index + 3] = 0x0f;

        UFD_WRITE(FfileHandle, FATStartAddr1 + HAHA.ClusterNo / 128, 1, ubDataBuf, 0);
        UFD_WRITE(FfileHandle, FATStartAddr2 + HAHA.ClusterNo / 128, 1, ubDataBuf, 0);
    }

}

// 主函数
VOID __cdecl main(int argc, char* argv[])
{
    printf("---------------------------------------------------------------------------------------------------\n");
    printf(" <<%s %x.%x_%s, %s>>\n", VERSION_HANDLE, VERSION_H, VERSION_L, __DATE__, __TIME__);
    printf("---------------------------------------------------------------------------------------------------\n");

    // 参数检查
    if (argc < 2)
    {
        printf("Usage:  %s <drive:>\n", argv[0]);
        return;
    }

    //// 《---------------------------------------------------------Create FileHandle ---------------------------------------------------------------》
    DWORD accessMode = 0; // 访问模式
    DWORD shareMode = 0; // 共享模式
    DWORD ReturnBytes = 0; // 返回的字节数
    DWORD errorCode = 0; // 错误代码
    HANDLE fileHandle = NULL; //文件句柄
    UCHAR string_path[25]; // 存储驱动器路径
    UCHAR ubDataBuf[512] = { 0 }; // 数据缓冲区


    strncpy_s((char*)string_path, 25, "\\\\.\\", 24);
    strncat_s((char*)string_path, 25, argv[1], 24);

    printf("Current Drive: %s\n", string_path);
    printf("\n");

    // 设置文件访问和共享模式
    shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    accessMode = GENERIC_WRITE | GENERIC_READ;


    // 打开驱动器
    fileHandle = CreateFile((PCHAR)string_path,
        accessMode,
        shareMode,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        printf("Error opening %s. Error: %d\n", argv[1], errorCode = GetLastError());
        return;
    }

    // 锁定卷
    DeviceIoControl(fileHandle,
        FSCTL_LOCK_VOLUME,
        NULL, 0,
        NULL, 0,
        &ReturnBytes,
        false);

    std::string searchString = "test"; //用于寻找包含该字符串的文件或目录
    std::string delete_file_path = "E:/TEST/TEST.TXT"; // 要删除的文件或文件夹名

    std::string create_file_path = "E:"; // 要创建的文件路径

    std::string source_file_path = "E:/TEST/TEST.TXT"; // 要复制的源文件路径
    std::string destination_file_path = "E:"; // 要复制的目标路径

    std::string recover_file_path = "E:/TEST1/HAHA1.JPG"; // 要恢复的文件路径

    // 《-----------------------------------------------------------Parsing FAT32-------------------------------------------------------------------》
    FAT32 fat32(fileHandle,L"E:"); // FAT32的结构体

    //// 《---------------------------------------------------------Serach file or dir---------------------------------------------------------------》
    //fat32.SearchInPathVector(searchString);

    //// 《---------------------------------------------------------Delete file or dir---------------------------------------------------------------》
    //fat32.DeleteSpecifiedFile(fileHandle, delete_file_path);

    //// 《---------------------------------------------------------Create file or dir---------------------------------------------------------------》
    /*fat32.CreateFileOrDir(fileHandle, create_file_path);*/

    //// 《---------------------------------------------------------Copy file or dir-----------------------------------------------------------------》
    // fat32.CopyFileOrDir(fileHandle, source_file_path, destination_file_path);

    //// 《---------------------------------------------------------Recover file-----------------------------------------------------------------》
    // fat32.RecoverFile(fileHandle, recover_file_path);

    // 解锁卷
    DeviceIoControl(fileHandle,
        FSCTL_UNLOCK_VOLUME,
        NULL, 0,
        NULL, 0,
        &ReturnBytes,
        false);

    // 关闭文件句柄
    CloseHandle(fileHandle);

    system("pause"); // 暂停以查看输出

    return;
}
