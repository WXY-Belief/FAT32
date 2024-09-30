/**************************************************************************************************/
/*																							      */
/*					Copyright (c) 2018-2024 by Hosin Global Electronics Co., LTD.				  */
/*																								  */
/**************************************************************************************************/
#include "spti.h"

// ��vector<UCHAR> ת��Ϊ ULONG
ULONG vectorToULong(const std::vector<unsigned char>& vec) {
    ULONG size = 0;
    if (vec.size() >= 4) {
        // �� 4 ���ֽںϲ�Ϊ ULONG������С���ֽ���
        size = (static_cast<unsigned long>(vec[0]) |
            (static_cast<unsigned long>(vec[1]) << 8) |
            (static_cast<unsigned long>(vec[2]) << 16) |
            (static_cast<unsigned long>(vec[3]) << 24));
    }
    return size;
}

// ��ʽ���ļ���СΪ�ַ���
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

// ��ӡ��ȡ��Sector������
void PrintData(UCHAR* BUFFER, string name) {
    // ��ӡ��ȡ������
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

// ����SCSI����ĺ���
BOOL IssueCDB(HANDLE fileHandle, UCHAR ucCDBLength, PUCHAR CDBData, UCHAR ucDataIn, PVOID pDataBuf, ULONG usBufLen)
{
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    BOOL status = 0;
    ULONG length = 0, returned = 0;
    UCHAR ucCount;

    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER)); // ��սṹ��

    // ����SCSI_PASS_THROUGH_DIRECT�ṹ��Ĳ���
    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = ucCDBLength;
    sptdwb.sptd.DataIn = ucDataIn; // ���ݴ��䷽���������뻹�����
    sptdwb.sptd.SenseInfoLength = 24; // Sense��Ϣ����
    sptdwb.sptd.DataTransferLength = usBufLen; // ���ݴ��䳤��
    sptdwb.sptd.TimeOutValue = 9000; // ��ʱֵ
    sptdwb.sptd.DataBuffer = pDataBuf; // ���ݻ�����
    sptdwb.sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf); // Sense��Ϣ��ƫ����

    // ��CDB�������ݿ������ṹ����
    for (ucCount = 0; ucCount < 16; ucCount++)
    {
        sptdwb.sptd.Cdb[ucCount] = CDBData[ucCount];
    }

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    // ����IOCTL����
    status = DeviceIoControl(fileHandle,
        IOCTL_SCSI_PASS_THROUGH_DIRECT, // IOCTL����
        &sptdwb, // ���뻺����
        length,
        &sptdwb, // ���������
        length,
        &returned, // ʵ�ʷ��ص��ֽ���
        FALSE); // ��ʹ���ص�IO


    //printf("Sense Data: ");
    //for (int i = 0; i < sptdwb.sptd.SenseInfoLength; i++) {
    //    printf("%02X ", sptdwb.ucSenseBuf[i]);
    //}
    //printf("\n");
    return status;
}

// ��ȡ���ݺ���
UCHAR UFD_READ(HANDLE fileHandle, ULONG ulLBA, USHORT usLen, PUCHAR pucBuffer, UCHAR ucLUN)
{
    /**
     * @brief ͨ��SCSIָ���ȡָ��LBA��ַ�ϵ����ݿ顣
     *
     * @param fileHandle �ļ���������ڷ��ʺͲ����豸��
     * @param ulLBA Ҫ��ȡ���߼����ַ��LBA���������ݿ��ڴ洢�����ϵ�λ�á�
     * @param usLen Ҫ��ȡ�����ݿ�����ÿ�����ݿ�ͨ��Ϊ512�ֽڡ�
     * @param pucBuffer ָ��洢��ȡ���ݵĻ�������ָ�롣
     * @param ucLUN �߼���Ԫ�ţ�LUN�������ڱ�ʶ���߼���Ԫ�豸�ľ��嵥Ԫ��
     *
     * @return UCHAR ����״̬��ͨ�����ز�������״̬����0��ʾ�ɹ���0��ʾʧ�ܣ���
     */
    UCHAR ucCDB[16];
    BOOL status;
    ULONG Len = (ULONG)usLen * 512; // �������ݳ���

    ZeroMemory(ucCDB, 16); // ���CDB����
    ucCDB[0] = SCSIOP_READ; // ����SCSI��ȡ����
    ucCDB[1] = ucLUN << 5; // ����LUN
    ucCDB[2] = (UCHAR)(ulLBA >> 24); // ����LBA��λ
    ucCDB[3] = (UCHAR)(ulLBA >> 16); // ����LBA��λ
    ucCDB[4] = (UCHAR)(ulLBA >> 8); // ����LBA��λ
    ucCDB[5] = (UCHAR)(ulLBA); // ����LBA��λ
    ucCDB[7] = (UCHAR)(usLen >> 8); // ���ó��ȸ�λ
    ucCDB[8] = (UCHAR)(usLen); // ���ó��ȵ�λ

    // ����SCSI�����ȡ����
    status = IssueCDB(fileHandle, CDB10GENERIC_LENGTH, ucCDB, SCSI_IOCTL_DATA_IN, pucBuffer, Len);
    if (!status)
    {
        printf("Error in ufd read\n"); // ��ȡʧ��
    }

    if (DEBUG_Flag) { cout << "read:" << status << endl; }
    return status;
}

// д�����ݺ���
UCHAR UFD_WRITE(HANDLE fileHandle, ULONG ulLBA, USHORT usLen, PUCHAR pucBuffer, UCHAR ucLUN)
{
    UCHAR ucCDB[16];
    BOOL status;
    ULONG Len = (ULONG)usLen * 512; // �������ݳ���

    ZeroMemory(ucCDB, 16); // ���CDB����
    ucCDB[0] = SCSIOP_WRITE; // ����SCSIд������
    ucCDB[1] = ucLUN << 5; // ����LUN
    ucCDB[2] = (UCHAR)(ulLBA >> 24); // ����LBA��λ
    ucCDB[3] = (UCHAR)(ulLBA >> 16); // ����LBA��λ
    ucCDB[4] = (UCHAR)(ulLBA >> 8); // ����LBA��λ
    ucCDB[5] = (UCHAR)(ulLBA); // ����LBA��λ
    ucCDB[7] = (UCHAR)(usLen >> 8); // ���ó��ȸ�λ
    ucCDB[8] = (UCHAR)(usLen); // ���ó��ȵ�λ
    

    // ����SCSI���д������
    status = IssueCDB(fileHandle, CDB10GENERIC_LENGTH, ucCDB, SCSI_IOCTL_DATA_OUT, pucBuffer, Len);

    if (DEBUG_Flag) {
        if (!status)
        {
            DWORD error = GetLastError();
            printf("Error in ufd write, Error: %lu\n", error); // д��ʧ��
        }
        else {
            cout << "Write Successed, status: " << status << endl;
        }

    }
    
    return status;
}

// �޸�FINFO
void FAT32::ModifyClusterInfo(HANDLE FfileHandle, ULONG number, ULONG next_cluster, int is_add_flag) {
    UCHAR ubDataBuf[512] = { 0 }; // ���ݻ�����

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

//����MBR���ҵ�ÿ��Partition����Ϣ
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

//����DBR, ��ȡBPB��Ϣ
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

// ��ĳ����������ı���ƴ������
vector<UCHAR> ParsingInfo(UCHAR *ubDataBu, int start, int length) {
    vector<UCHAR> info;

    for (int i = 0; i < length; i++) {
        info.insert(info.end(), ubDataBu[start + i]);
    }
    return info;
}

// ��ӡ vector<UCHAR> �е�Ԫ��
void printVectorUCHAR(const vector<UCHAR>& vec) {
    cout << "Vector<UCHAR> elements: ";
    for (UCHAR elem : vec) {
        cout << "0x" << hex << setw(2) << setfill('0') << (int)elem << " ";
    }
    cout << endl;
}

// ��16����תΪ�ַ������͵�ʱ��
string HexToTime(vector<UCHAR> Date, vector<UCHAR> time = {}) {
    if (Date.size() < 2) {
        // ��� Date Ϊ�ջ򳤶Ȳ��㣬���ؿ��ַ���
        return "";
    }

    int year = 1980, month = 1, day = 1;  // Ĭ��ֵ

    if (!Date.empty()) {
        ULONG this_date = Date[1] << 8 | Date[0];
        year = (this_date >> 9) + 1980;
        month = (this_date >> 5) & 0x0F;
        day = this_date & 0x1F;
    }

    // ��� time Ϊ�ջ򳤶Ȳ��㣬���ؽ��������ڵ��ַ���
    if (time.empty() || time.size() < 2) {
        char date_buffer[11]; // �����ڲ��ֵĸ�ʽ��������
        snprintf(date_buffer, sizeof(date_buffer), "%04d-%02d-%02d", year, month, day);
        return string(date_buffer);
    }

    int hour = 0, minute = 0, second = 0; // Ĭ��ֵ
    ULONG this_time = time[1] << 8 | time[0];
    hour = this_time >> 11;
    minute = (this_time >> 5) & 0x3F;
    second = (this_time & 0x1F) * 2;

    // ��ʽ��Ϊ��׼����ʱ���ַ���
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);

    return string(buffer);
}

// ��ʮ������ֵת��Ϊ��Ӧ���ַ��������ַ���
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


//����Ŀ¼�����������νṹ������
void FAT32::ParsingDir(HANDLE pfileHandle, ULONG DataBaseAddress, TreeNode* Node) {
    UCHAR DataBuffer[512] = { 0 }; // ���ݻ�����
    int flag = 1;//�Ƿ���Ҫ��ȡ��һ��Sector
    int index; //ÿ���ļ�����ʼ����

    File file = File(); // �ļ���Ϣ

    // ������Ŀ¼
    while (flag) {
        UFD_READ(pfileHandle, DataBaseAddress, 1, DataBuffer, 0);
        DataBaseAddress = DataBaseAddress + 1;
        
        index = 0;
        
        while (index < 512) {
            UCHAR FileMark = DataBuffer[index];
            UCHAR LongFileNameFlag = DataBuffer[index+11];

            if (FileMark == 0xE5 || FileMark == 0x2E) { // �����E5������ļ��Ѿ�ɾ����, 2E��Ŀ¼����һ���͵�ǰĿ¼
                index = index + 32;
                continue;
            }
            else if (FileMark == 0x00) {
                flag = 0;
                break;
            }
            else if(LongFileNameFlag == 0x0F){ //�����0F�����ʱ��ȡ���ǳ��ļ���
                vector<UCHAR> temp_1 = ParsingInfo(DataBuffer, index + 1, 10);
                vector<UCHAR> temp_2 = ParsingInfo(DataBuffer, index + 14, 12);
                vector<UCHAR> temp_3 = ParsingInfo(DataBuffer, index + 28, 4);


                std::vector<int> mergedVec;
                mergedVec.insert(mergedVec.end(), temp_1.begin(), temp_1.end());
                mergedVec.insert(mergedVec.end(), temp_2.begin(), temp_2.end());
                mergedVec.insert(mergedVec.end(), temp_3.begin(), temp_3.end());

                file.FileName.insert(file.FileName.begin(), mergedVec.begin(), mergedVec.end());
            }
            else { //��ʱ��ȡ���Ƕ��ļ���
                if (file.FileName.empty()) { // �ļ���Ϊ��˵�����ļ��Ƕ��ļ��������ն��ļ������н�����
                    //��ȡ�ļ���
                    vector<UCHAR> short_file_name = ParsingInfo(DataBuffer, index, 8);
                    file.FileName.insert(file.FileName.begin(), short_file_name.begin(), short_file_name.end());
                    file.FileNameString = hexToUtf8(file.FileName); //������ʵ���ļ���
                }

                file.FileNameString = hexToUtf8(file.FileName); //������ʵ���ļ���

                file.Extension = ParsingInfo(DataBuffer, index + 8, 3);
                file.ExtensionString = hexToUtf8(file.Extension); //������ʵ����չ��

                file.Attribution = DataBuffer[index + 11];
                file.Reserved = DataBuffer[index + 12];
                file.CR = DataBuffer[index + 13];

                file.CreationTime = ParsingInfo(DataBuffer, index + 14, 2);
                file.CreationDate = ParsingInfo(DataBuffer, index + 16, 2);
                file.CreationTimeString = HexToTime(file.CreationDate, file.CreationTime);// �ļ��Ĵ���ʱ��

                file.AccessDate = ParsingInfo(DataBuffer, index + 18, 2);
                file.AccessDateString = HexToTime(file.AccessDate);

                vector<UCHAR> hclusterno = ParsingInfo(DataBuffer, index + 20, 2);
                vector<UCHAR> lclusterno = ParsingInfo(DataBuffer, index + 26, 2);
                file.ClusterNo = hclusterno[1] << 24 | hclusterno[0] << 16 | lclusterno[1] << 8 | lclusterno[0]; // �ļ���ʼ�غ�

                file.ModifyTime = ParsingInfo(DataBuffer, index + 22, 2);
                file.ModifyDate = ParsingInfo(DataBuffer, index + 24, 2);
                file.ModifyTimeString = HexToTime(file.ModifyDate, file.ModifyTime);  // �ļ����޸�ʱ��

                file.Size = ParsingInfo(DataBuffer, index + 28, 4);
                file.SizeString = formatFileSize(vectorToULong(file.Size));

                // �洢�����ṹ��
                TreeNode* this_node = new TreeNode(1);
                if (file.Attribution != 0x10)
                {
                    this_node->IsFolderFlag = 0;
                }
                this_node->FileInfo = file;
                this_node->ParentNode = Node;
                Node->Children.push_back(this_node);

                //���ýṹ��ȴ���һ���ļ�
                file.reset(); 
            }
            index = index + 32;
        }
    }
}

// ʹ�ò�α�����ÿ��Ŀ¼����ʼ�غŶ�ȡ��Ŀ¼�������ļ���Ŀ¼��
void FAT32::CreateCataloge(HANDLE FileHandle) {
    if (Root == nullptr) return;

    // ����һ������
    queue<TreeNode*> q; 

    // ���ڵ����
    q.push(Root); 
    
    while (!q.empty()) {
        TreeNode* Node = q.front(); 
        q.pop();                   // ����һ��Ԫ��

        // Ѱ�Ҹ�Ŀ¼�������ļ���Ŀ¼
        ParsingDir(FileHandle, DataRegionAddr + (Node->FileInfo.ClusterNo - 2) * dbr->bpb.SectorsPerCluster, Node);

        // ����Ŀ¼�µ�����Ŀ¼���
        for (auto child : Node->Children) {
            if (child->IsFolderFlag == 1) {
                q.push(child);
            }
        }
    }
}

// ��ӡ����Ŀ¼�ṹ
void FAT32::PrintDirectory(TreeNode* Node, int level) {
    if (Node == nullptr) return;

    wstring name;
    string type;

    // ��ӡ��ǰĿ¼�����ƣ�ǰ������ʵ������� '-'
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

    // �ݹ��ӡ������Ŀ¼
    for (TreeNode* child : Node->Children) {
        PrintDirectory(child, level + 1);
    }
}

// ���ַ�תΪСд
string ToLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// ��ȡÿ���ļ������ļ��еľ���·��
void FAT32::TraverseTree(TreeNode* Node, string ParentPath, ULONG ParentClusterNo, vector<vector<string>>& PathVector) {
    if (!Node) return;

    // ��ǰ�ڵ��·��
    string currentPath = ParentPath.empty() ? Node->FileInfo.FileNameString : ParentPath + "/" + Node->FileInfo.FileNameString;

    // ȷ���ڵ�����
    string nodeType = Node->IsFolderFlag ? "Directory" : "File";

    // �����ļ�������Ŀ¼���ڵ����͡���Ŀ¼�Ĵغţ�����ɾ��������ǰ�ڵ�Ĵغţ����������ļ������ļ��У�
    if (Node->IsFolderFlag) {
        PathVector.push_back({ Node->FileInfo.FileNameString, ParentPath, nodeType });

        // �ݹ�����ӽڵ�
        for (TreeNode* child : Node->Children) {
            TraverseTree(child, currentPath, Node->FileInfo.ClusterNo, PathVector);
        }
    }
    else {
        PathVector.push_back({ Node->FileInfo.FileNameString + "." + Node->FileInfo.ExtensionString, ParentPath, nodeType });
    }
}

// �������������ָ���ַ������ļ����ļ���
void FAT32::SearchInPathVector(const std::string& SearchString) {
    printf("---------------------------------------------------------------------------------------------------\n");
    cout << "The Result about Searching String:" << SearchString << endl;
    printf("---------------------------------------------------------------------------------------------------\n");

    vector<vector<string>> result;
    string lowerSearchString = ToLower(SearchString);

    for (const auto& entry : FileAbsolutePath) {
        std::string name = ToLower(entry[0]);  // �ļ���Ŀ¼��
        std::string parentPath = ToLower(entry[1]);  // ��Ŀ¼·��
        std::string nodeType = ToLower(entry[2]);  // �ڵ�����

        // ����ļ�����Ŀ¼���Ƿ���������ַ���
        if (string(name).find(lowerSearchString) != std::string::npos) {
            result.push_back(entry);  // ��ƥ�����Ŀ����������
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

// ���ݡ�/���ָ�String
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

// Ѱ��ĳ���ļ�����Ŀ¼�ĸ�Ŀ¼
TreeNode* FAT32::FindNode(string FilePath) {
    if (Root == nullptr) return NULL;

    // �����ַ���
    vector<std::string> parts = DevideString(FilePath);

    if (parts.empty()) return NULL;

    TreeNode* Node = Root; //�ҵ��ĸ��ڵ�
    unsigned int index = 1;
    int flag;

    // �����ļ�ʱ�����ڵ��µ��ļ�
    cout << Root->FileInfo.FileNameString << " " << FilePath << endl;
    if (Root->FileInfo.FileNameString == FilePath) {
        return Root;
    }

    while (Node != NULL) {
        flag = 1;
        for (auto child : Node->Children) { //������ǰ�ڵ�ĺ��ӽڵ�
            string name = child->IsFolderFlag ? child->FileInfo.FileNameString : child->FileInfo.FileNameString + "." + child->FileInfo.ExtensionString;

            // ���ӽڵ���index��Ӧ��·�����
            if (name == parts[index]) {
                index += 1;
                flag = 0;
                Node = child;
                break;
            }
        }

        if (flag) {//û�ҵ��ļ����˳�
            Node = NULL;
        }
        if (index == parts.size()) { // ·��Ѱ����ϣ��˳�
            return Node;
        }
    }
    return NULL;
}

// ��ɾ���ļ����ļ�����־��ΪE5
ULONG FAT32::DeleteFileOrDirInCluster(HANDLE FfileHandle, string DeleteFileName, TreeNode* ParentNode) {
    ULONG DataBaseAddress = DataRegionAddr + (ParentNode->FileInfo.ClusterNo - 2) * dbr->bpb.SectorsPerCluster;

    UCHAR DataBuffer[512] = { 0 }; // ���ݻ�����
    int flag = 1;//�Ƿ���Ҫ��ȡ��һ��Sector
    int index; //ÿ���ļ�����ʼ����

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

            if (file_flag == 0xE5 || file_flag == 0x2E) { // �����E5������ļ��Ѿ�ɾ����
                index = index + 32;
                continue;
            }
            else if (file_flag == 0x00) {
                flag = 0;
                break;
            }
            else if (file_long_flag == 0x0F) { //�����0F�����ʱ��ȡ���ǳ��ļ���
                vector<UCHAR> temp_1 = ParsingInfo(DataBuffer, index + 1, 10);
                vector<UCHAR> temp_2 = ParsingInfo(DataBuffer, index + 14, 12);
                vector<UCHAR> temp_3 = ParsingInfo(DataBuffer, index + 28, 4);


                std::vector<int> mergedVec;
                mergedVec.insert(mergedVec.end(), temp_1.begin(), temp_1.end());
                mergedVec.insert(mergedVec.end(), temp_2.begin(), temp_2.end());
                mergedVec.insert(mergedVec.end(), temp_3.begin(), temp_3.end());

                filename.insert(filename.begin(), mergedVec.begin(), mergedVec.end());
            }
            else { // ���ļ����ļ���������
                if (filename.empty()) { // �ļ���Ϊ��˵�����ļ��Ƕ��ļ��������ն��ļ������н�����
                    //��ȡ�ļ���
                    vector<UCHAR> short_file_name = ParsingInfo(DataBuffer, index, 8);
                    filename.insert(filename.begin(), short_file_name.begin(), short_file_name.end());
                }

                filename_string = hexToUtf8(filename); //������ʵ���ļ���

                extension = ParsingInfo(DataBuffer, index + 8, 3);
                extension_string = hexToUtf8(extension); //������ʵ����չ��

                vector<UCHAR> hclusterno = ParsingInfo(DataBuffer, index + 20, 2);
                vector<UCHAR> lclusterno = ParsingInfo(DataBuffer, index + 26, 2);

                name = DataBuffer[index + 11] == 0x10 ? filename_string : filename_string + "." + extension_string;

                if (name == DeleteFileName) { // �ҵ���Ӧ���ļ���λ����
                    clusterno = hclusterno[1] << 24 | hclusterno[0] << 16 | lclusterno[1] << 8 | lclusterno[0]; // �ļ���ʼ�غ�

                    // �ļ����޸ı�־λΪ0xE5
                    int xidenx = index + 11;
                    do {
                        DataBuffer[index] = 0xE5;
                        index -= 32;
                        xidenx -= 32;
                    } while (DataBuffer[xidenx] == 0x0F);

                    UFD_WRITE(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);
                    break;
                }

                filename.clear(); //����ļ���
                extension.clear(); //�����չ��
            }
            index = index + 32;
        }

        DataBaseAddress = DataBaseAddress + 1;
    }
    return clusterno;
}

//��FAT��ɾ���ļ���ClusterNo
void FAT32::DeleteFileClusterFromFAT(HANDLE FfileHandle, vector<ULONG> Clusters) {
    UCHAR DataBuffer[512] = { 0 }; // ���ݻ�����
    ULONG next_item;
    int start_index; // �غŵ���Sector�еĿ�ʼλ��

    for (auto clusterno : Clusters) {
        UFD_READ(FfileHandle, FATStartAddr1 + clusterno / 128, 1, DataBuffer, 0);

        start_index = int(clusterno % 128) * 4;
        next_item = DataBuffer[start_index + 3] << 24 | DataBuffer[start_index + 2] << 16 | DataBuffer[start_index + 1] << 8 | DataBuffer[start_index];

        while (next_item != 0x0fffffff) {
            // �����ļ���Ӧ��FAT ietm��ΪĬ��ֵ
            DataBuffer[start_index] = 0x00;
            DataBuffer[start_index + 1] = 0x00;
            DataBuffer[start_index + 2] = 0x00;
            DataBuffer[start_index + 3] = 0x00;
            UFD_WRITE(FfileHandle, FATStartAddr1 + clusterno / 128, 1, DataBuffer, 0);
            UFD_WRITE(FfileHandle, FATStartAddr2 + clusterno / 128, 1, DataBuffer, 0);

            //��ȡ���ļ�����һ��ClusterNo
            UFD_READ(FfileHandle, FATStartAddr1 + next_item / 128, 1, DataBuffer, 0);
            start_index = int(next_item % 128) * 4;
            next_item = DataBuffer[start_index + 3] << 24 | DataBuffer[start_index + 2] << 16 | DataBuffer[start_index + 1] << 8 | DataBuffer[start_index];
        }

        // ���һ����ɾ��
        DataBuffer[start_index] = 0x00;
        DataBuffer[start_index + 1] = 0x00;
        DataBuffer[start_index + 2] = 0x00;
        DataBuffer[start_index + 3] = 0x00;
        UFD_WRITE(FfileHandle, FATStartAddr1 + clusterno / 128, 1, DataBuffer, 0);
        UFD_WRITE(FfileHandle, FATStartAddr2 + clusterno / 128, 1, DataBuffer, 0);
    }
}

// ɾ��ָ�����ļ��л����ļ�
void FAT32::DeleteSpecifiedFile(HANDLE FfileHandle, string DeleteFilePath) {
    printf("---------------------------------------------------------------------------------------------------\n");
    cout << "The Result Deleting String:" << DeleteFilePath << endl;
    printf("---------------------------------------------------------------------------------------------------\n");

    //Ѱ�Ҹýڵ�
    TreeNode* DeleteFileNode = FindNode(DeleteFilePath);
    string DeleteFileName = DevideString(DeleteFilePath).back(); //ɾ���ļ����ļ���
    vector<ULONG> FATStartClusterNo; // ɾ���ļ��Ĵغ�

    //�ж��Ƿ����ҵ����ļ����Լ������ļ������ļ���
    if (DeleteFileNode == NULL) {
        cout << "No find File" << endl;
    }
    else {
        cout << "Finding File Successed. Parent Dir is " << DeleteFileNode->ParentNode->FileInfo.FileNameString;

        //���������ļ�����Ŀ¼����ͬ�Ĳ�����Directory or File
        if (DeleteFilePath.find('.') != std::string::npos) { // ִ��ɾ���ļ���Ӧ�Ĳ���
            // ɾ����Ӧ���ļ�
            ULONG FileStartCluster = DeleteFileOrDirInCluster(FfileHandle, DevideString(DeleteFilePath).back(), DeleteFileNode->ParentNode);

            if (FileStartCluster != 0) {
                FATStartClusterNo.push_back(FileStartCluster);
            }
        }
        else {// ִ��ɾ���ļ��ж�Ӧ�Ĳ���
            // ���Ƚ���ǰ�ļ���ɾ��
            ULONG FileStartCluster = DeleteFileOrDirInCluster(FfileHandle, DevideString(DeleteFilePath).back(), DeleteFileNode->ParentNode);

            queue<TreeNode*> q; // ����һ������
            // ��Ҫɾ�����ļ��нڵ����
            q.push(DeleteFileNode);

            while (!q.empty()) {
                TreeNode* Node = q.front();
                q.pop();                   // ����һ��Ԫ��

                for (auto child : Node->Children) {
                    string ThisDeleteFileName = child->IsFolderFlag ? child->FileInfo.FileNameString : child->FileInfo.FileNameString + "." + child->FileInfo.ExtensionString;
                    ULONG FileStartCluster = DeleteFileOrDirInCluster(FfileHandle, ThisDeleteFileName, Node);

                    if (child->IsFolderFlag == 0 && FileStartCluster != 0) { // ɾ���ļ��������俪ʼ�ؼ���FATstartcluster��
                        FATStartClusterNo.push_back(FileStartCluster);
                    }
                    else {
                        q.push(child);
                    }
                }
            }
        }

         //ɾ��FAT���ж�Ӧ�Ĵ�
         if (!FATStartClusterNo.empty()) {
             DeleteFileClusterFromFAT(FfileHandle, FATStartClusterNo);
         }

         // �޸ķ�����Ϣ
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

// ��ȡ��ǰʱ�䲢����ת��Ϊʹ�á�-�����ӵ��ַ���
string getCurrentTimeString() {
    // ��ȡ��ǰʱ���
    auto now = std::chrono::system_clock::now();

    // ת��Ϊtime_t���ͣ��Ի�ȡ�뼶ʱ���
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // ����һ�� tm �ṹ���洢����ʱ��
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);  // ʹ�� localtime_s �滻 localtime

    // ʹ��ostringstream��ʱ���ʽ��Ϊ�ַ�����������ʱ�䲿��
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%H-%M-%S");  // ������ʱ�䲿�֣����á�-������

    std::cout << oss.str() << std::endl;
    return oss.str();
}

// ��ʱ��ת��Ϊ FAT ʱ���ʽ
uint16_t getFatTime(struct tm* tstruct) {
    return (tstruct->tm_hour << 11) | (tstruct->tm_min << 5) | (tstruct->tm_sec / 2);
}

// ������ת��Ϊ FAT ���ڸ�ʽ
uint16_t getFatDate(struct tm* tstruct) {
    return ((tstruct->tm_year - 80) << 9) | ((tstruct->tm_mon + 1) << 5) | tstruct->tm_mday;
}

// ��ת�ֽ�˳����ʵ��С�˴洢
uint16_t toLittleEndian(uint16_t value) {
    return (value >> 8) | (value << 8);
}

// ��ȡ��ǰʱ�䣬�����������Ϣ������Ҫ���浽Data�������Ϣ
vector<UCHAR> CreateFileName(ULONG FileCluster) {
    vector<UCHAR> FileName(32, 0);

    // ʹ�õ�ǰʱ����Ϊ�ļ����ļ�����
    string timeStr = getCurrentTimeString();
    replace(timeStr.begin(), timeStr.end(), '-', '_'); // �滻'-'Ϊ'_'

    string Name = timeStr.substr(0, 8); // ȡǰ8���ַ���Ϊ�ļ���
    string Ext = "TXT"; // ������չ��Ϊ txt

    // ����ļ�������չ����SFN�ṹ��
    for (string::size_type i = 0; i < 8; ++i) {
        if (i < Name.size()) {
            FileName[i] = Name[i];
        } else {
            FileName[i] = 0x20; // �ÿո����
        }
    }
    for (string::size_type i = 0; i < 3; ++i) {
        if (i < Ext.size()) {
            FileName[8 + i] = Ext[i];
        } else {
            FileName[8 + i] = 0x20; // �ÿո����
        }
    }

    FileName[11] = 0x20; // �ļ�����

    // ��ȡ��ǰ�ļ��Ĵ���ʱ����޸�ʱ��
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

// ��FAT����Ѱ���ܹ����������ļ����еĴ�
ULONG FAT32::FindFreeCluster(HANDLE FfileHandle) {
    ULONG FreeCluster = 2;
    UCHAR ubDataBuf[512] = { 0 };
    ULONG Temp;
    unsigned int offset;


    // ��ȡFAT����Ϣ
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

// �����ļ����ļ���
void FAT32::CreateFileOrDir(HANDLE FfileHandle, string CreateFilePath) {
    printf("---------------------------------------------------------------------------------------------------\n");
    cout << "The Result Creating String:" << CreateFilePath << endl;
    printf("---------------------------------------------------------------------------------------------------\n");

    UCHAR DataBuffer[512] = { 0 }; // ���ݻ�����
    unsigned int start_index =0;
    ULONG DataBaseAddress ;

    // Ѱ�Ҹýڵ�
    TreeNode* CreateFileNode = FindNode(CreateFilePath);
    if (CreateFileNode == NULL) {
        cout << "No find File" << endl;
    }
    else {
        // �����ļ���Ϣ���丸Ŀ¼�´����ļ���
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

        // �������ļ�����16���Ƹ�ʽ
        vector<UCHAR> sfnPtr = CreateFileName(0x00);

        // �����Ӧλ�ò�д����������
        for (int i = 0; i < 32; i++) {
            DataBuffer[start_index + i] = sfnPtr[i];
        }

        //д����������
        UFD_WRITE(FfileHandle, DataBaseAddress, 1, DataBuffer, 0);

        cout << "Create Successed" << endl;
    }
}

// ��FAT���ҵ��ļ����еĴ�
vector<ULONG> FAT32::FindFileClusterFromFAT(HANDLE FfileHandle, ULONG StartClusterNo) {
    UCHAR DataBuffer[512] = { 0 }; // ���ݻ�����
    ULONG next_item;
    int start_index; // �غŵ���Sector�еĿ�ʼλ��
    vector<ULONG> Clusters;
    Clusters.push_back(StartClusterNo);

    UFD_READ(FfileHandle, FATStartAddr1 + StartClusterNo / 128, 1, DataBuffer, 0);
    start_index = int(StartClusterNo % 128) * 4;
    next_item = DataBuffer[start_index + 3] << 24 | DataBuffer[start_index + 2] << 16 | DataBuffer[start_index + 1] << 8 | DataBuffer[start_index];

    while (next_item != 0x0fffffff) {
        Clusters.push_back(next_item);

        //��ȡ���ļ�����һ��ClusterNo
        UFD_READ(FfileHandle, FATStartAddr1 + next_item / 128, 1, DataBuffer, 0);
        start_index = int(next_item % 128) * 4;
        next_item = DataBuffer[start_index + 3] << 24 | DataBuffer[start_index + 2] << 16 | DataBuffer[start_index + 1] << 8 | DataBuffer[start_index];
    }

    return Clusters;
}

// �ҵ��ļ������ļ��ж�Ӧ����Ϣ�Լ��ڸ�Ŀ¼�µı���
vector<UCHAR> FAT32::FindFileOrDirCode(HANDLE FfileHandle, string FileName, TreeNode* ParentNode) {
    ULONG DataBaseAddress = DataRegionAddr + (ParentNode->FileInfo.ClusterNo - 2) * dbr->bpb.SectorsPerCluster;
    UCHAR DataBuffer[512] = { 0 }; // ���ݻ�����
    int flag = 1;//�Ƿ���Ҫ��ȡ��һ��Sector
    int index; //ÿ���ļ�����ʼ����

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

            if (file_flag == 0xE5 || file_flag == 0x2E) { // �����E5������ļ��Ѿ�ɾ����
                index = index + 32;
                continue;
            }
            else if (file_flag == 0x00) {
                flag = 0;
                break;
            }
            else if (file_long_flag == 0x0F) { //�����0F�����ʱ��ȡ���ǳ��ļ���
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
            else { // ���ļ����ļ���������
                if (filename.empty()) { // �ļ���Ϊ��˵�����ļ��Ƕ��ļ��������ն��ļ������н�����

                    //��ȡ�ļ���
                    vector<UCHAR> short_file_name = ParsingInfo(DataBuffer, index, 8);
                    filename.insert(filename.begin(), short_file_name.begin(), short_file_name.end());
                }

                vector<UCHAR> all32 = ParsingInfo(DataBuffer, index, 32);
                Buf32.insert(Buf32.begin(), all32.begin(), all32.end());

                filename_string = hexToUtf8(filename); //������ʵ���ļ���
                extension = ParsingInfo(DataBuffer, index + 8, 3);
                extension_string = hexToUtf8(extension); //������ʵ����չ��
            
                
                name = DataBuffer[index + 11] == 0x10 ? filename_string : filename_string + "." + extension_string;
                if (name == FileName) { // �ҵ���Ӧ���ļ���λ����
                    flag = 0;
                    break;
                }
                filename.clear(); //����ļ���
                extension.clear(); //�����չ��
                Buf32.clear(); //���
            }
            index = index + 32;
        }

        DataBaseAddress = DataBaseAddress + 1;
    }
    return Buf32;
}

// ��FAT���ҵ��ܹ�����ļ��Ĵز��޸ı�־
vector<ULONG> FAT32::FindFreeClusterFromFATForCopy(HANDLE FfileHandle, ULONG ClusterNumber) {
    UCHAR DataBuffer[512] = { 0 }; // ���ݻ�����
    vector<ULONG> Clusters;
    ULONG number=0;
    ULONG Addr = FATStartAddr1;

    // Ѱ�ҿ��еĴ�
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

    // �޸����־
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

// ����Դ�ļ�Code��Ŀ��·����
void FAT32::CopySourceCodeToDestination(HANDLE FfileHandle, string FileName, TreeNode* DestinationNode, vector<UCHAR> SourceCode, ULONG SatrtCluster) {
    ULONG DataBaseAddress = DataRegionAddr + (DestinationNode->FileInfo.ClusterNo - 2) * dbr->bpb.SectorsPerCluster;
    UCHAR DataBuffer[512] = { 0 }; // ���ݻ�����
    int index; //ÿ���ļ�����ʼ����

    // �ж�Դ�ļ����ļ�������(���ļ��� /���ļ���)
    size_t SourceCodelength = SourceCode.size() / 32;

    // �޸��ļ����еĿ�ʼ�غ�
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

    // ��Ŀ��·����Ѱ�Һ��ʵ�λ�÷Ž�ȥ
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

    // ����ָ��λ��
    for (size_t i = 0; i < SourceCode.size(); i++) {
        DataBuffer[lastindex - (SourceCodelength) * 32 + i] = SourceCode[i];
    }

    UFD_WRITE(FfileHandle, DataBaseAddress - 1, 1, DataBuffer, 0);
}

// ����Դ�ļ����ݵ�Ŀ���ļ���
void FAT32::CopySourceFileContentToDestination(HANDLE FfileHandle, vector<ULONG> SourceClusters, vector<ULONG> DestinationClusters) {
    ULONG SourceAddress = DataRegionAddr;
    ULONG DestinationAddress = DataRegionAddr;
    UCHAR pubDataBuf[512] = { 0 }; // ���ݻ�����

    for (size_t i = 0; i < SourceClusters.size(); i++) {
        for (ULONG j = 0; j < dbr->bpb.SectorsPerCluster; j++)
        {
            SourceAddress = SourceAddress + (SourceClusters[i]-2) * dbr->bpb.SectorsPerCluster + j;
            DestinationAddress = DestinationAddress + (DestinationClusters[i]-2) * dbr->bpb.SectorsPerCluster + j;
            printf("SourceAddress: %x, DestinationAddress: %x\n", SourceAddress, DestinationAddress);

            // ����Դ����
            UFD_READ(FfileHandle, SourceAddress, 1, pubDataBuf, 0);

            //д��Ŀ���ļ�
            UFD_WRITE(FfileHandle, DestinationAddress, 1, pubDataBuf, 0);
        }
    }
}

// �����ļ������ļ��е�����һ��Ŀ¼��
void FAT32::CopyFileOrDir(HANDLE FfileHandle, string SourceFilePath, string DestinationFilePath) {
    //�ж�·���Ƿ����
    TreeNode* SourceNode = FindNode(SourceFilePath);
    TreeNode* DestinationNode = FindNode(DestinationFilePath);

    if (DestinationNode != NULL && SourceNode !=NULL) {//·������
        // �ж�Ŀ��·�����Ƿ����ͬ���ļ������ļ���
        vector<string> file_name = DevideString(SourceFilePath);

        // �ж�Ŀ��·�����Ƿ����ͬ���ļ������ļ���
        int flag = 1;
        for (auto child: DestinationNode->Children) {
            string name = child->IsFolderFlag ? child->FileInfo.FileNameString : child->FileInfo.FileNameString + "." + child->FileInfo.ExtensionString;

            if (file_name.back() == name) {
                flag = 0;
                break;
            }
        }
        if (flag) { //Ŀ��·���²�����ͬ���ļ������ļ���
            if (file_name.back().find(".") == std::string::npos) {//Ŀ¼

            }
            else {//�ļ�
                // ��FAT�����ҵ�Դ�ļ����еĴ�
                vector<ULONG> SourceClusters = FindFileClusterFromFAT(FfileHandle, SourceNode->FileInfo.ClusterNo);

                // ����Դ�ļ����丸Ŀ¼�µ��ļ���
                vector<UCHAR> Sourcefilecode = FindFileOrDirCode(FfileHandle, file_name.back(), SourceNode->ParentNode);

                // ��FAT���ҵ��ܹ�����ļ��Ĵز��޸ı�־
                vector<ULONG> DestinationClusters = FindFreeClusterFromFATForCopy(FfileHandle, SourceClusters.size());

                // �����ļ�Code��Ŀ��·����
                CopySourceCodeToDestination(FfileHandle, file_name.back(), DestinationNode, Sourcefilecode, DestinationClusters[0]);

                // �����ļ����ݵ�Ŀ��·����
                CopySourceFileContentToDestination(FfileHandle, SourceClusters, DestinationClusters);

                // �޸ķ�����Ϣ
                ModifyClusterInfo(FfileHandle, DestinationClusters.size(), DestinationClusters.back() + 1, 0);
            }
        }
        else {
            cout << "There is a file or folder with same name in Destination file path" << endl;
        }
    }
    else {//·��������
        if (DestinationNode == NULL) {
            cout << "Destination file path does not exist" << endl; 
        }

        if (SourceNode == NULL) {
            cout << "Source file path does not exist" << endl;
        }
        
    }
}

//��ȡ�ļ����ļ��еĸ�Ŀ¼�ľ���·��
string getLeadingPathComponent(const std::string& path) {
    // �ҵ����һ��·���ָ�����λ��
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        // ���û���ҵ�·���ָ���������һ�����ַ�������ʾû��ǰ��·����
        return "";
    }
    else {
        // �������һ���ָ���֮ǰ�Ĳ���
        return path.substr(0, pos);
    }
}

//utfתΪ16����
vector<UCHAR> Utf8Hex(const std::string& utf8String) {
    vector<UCHAR> hex; 
    for (unsigned char c : utf8String) {

        hex.push_back(static_cast<int>(c));
    }
    return hex;
}

// �ڸ�Ŀ¼��Ѱ���ļ���
File FAT32::FindFileNameForRecover(HANDLE FfileHandle, TreeNode* ParentNode, string filename) {
    ULONG DataBaseAddress = DataRegionAddr + (ParentNode->FileInfo.ClusterNo - 2)*dbr->bpb.SectorsPerCluster;
    UCHAR DataBuffer[512] = { 0 }; // ���ݻ�����
    int flag = 1;//�Ƿ���Ҫ��ȡ��һ��Sector
    int index; //ÿ���ļ�����ʼ����

    File file = File(); // �ļ���Ϣ
    string frontname = "";
    string endname = "";

    for (size_t i = 0; i < filename.size() - 4; i++) {
        frontname += filename[i];
    }

    for (size_t i = filename.size() - 3; i < filename.size(); i++) {
        endname += filename[i];
    }

    cout << frontname << " " << endname << endl;

    // ������Ŀ¼
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
            else if (file_long_flag == 0x0F) { //�����0F�����ʱ��ȡ���ǳ��ļ���
                vector<UCHAR> temp_1 = ParsingInfo(DataBuffer, index + 1, 10);
                vector<UCHAR> temp_2 = ParsingInfo(DataBuffer, index + 14, 12);
                vector<UCHAR> temp_3 = ParsingInfo(DataBuffer, index + 28, 4);


                std::vector<UCHAR> mergedVec;
                mergedVec.insert(mergedVec.end(), temp_1.begin(), temp_1.end());
                mergedVec.insert(mergedVec.end(), temp_2.begin(), temp_2.end());
                mergedVec.insert(mergedVec.end(), temp_3.begin(), temp_3.end());

                file.FileName.insert(file.FileName.begin(), mergedVec.begin(), mergedVec.end());
            }
            else { //��ʱ��ȡ���Ƕ��ļ���
                if (file.FileName.empty()) { // �ļ���Ϊ��˵�����ļ��Ƕ��ļ��������ն��ļ������н�����
                    //��ȡ�ļ���
                    vector<UCHAR> short_file_name = ParsingInfo(DataBuffer, index, 8);
                    file.FileName.insert(file.FileName.begin(), short_file_name.begin(), short_file_name.end());
                    file.FileNameString = hexToUtf8(file.FileName); //������ʵ���ļ���
                }

                file.FileNameString = hexToUtf8(file.FileName); //������ʵ���ļ���

                file.Extension = ParsingInfo(DataBuffer, index + 8, 3);
                file.ExtensionString = hexToUtf8(file.Extension); //������ʵ����չ��

                vector<UCHAR> hclusterno = ParsingInfo(DataBuffer, index + 20, 2);
                vector<UCHAR> lclusterno = ParsingInfo(DataBuffer, index + 26, 2);
                file.ClusterNo = hclusterno[1] << 24 | hclusterno[0] << 16 | lclusterno[1] << 8 | lclusterno[0]; // �ļ���ʼ�غ�

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

                //���ýṹ��ȴ���һ���ļ�
                file.reset();
            }
            index = index + 32;
        }
    }
    File file1 = File(); // �ļ���Ϣ
    return file1;
}

//�ָ�ɾ����ͼƬ
void FAT32::RecoverFile(HANDLE FfileHandle, string FilePath) {
    // Ѱ�Ҹ�Ŀ¼
    cout << getLeadingPathComponent(FilePath) << endl;
    TreeNode* ParentNode = FindNode(getLeadingPathComponent(FilePath));

    // �ȶ��ļ���
    string name = DevideString(FilePath).back();
    cout << name << endl;

    // ��ȡ��ʼ�غ��ļ���С
    File HAHA = FindFileNameForRecover(FfileHandle, ParentNode, name);

    if (HAHA.FileNameString == "") {
        cout << "no file" << endl;
    }
    else {
        cout << "Recover file" << endl;

        // ȥFAT ����˳���˳����ļ���ռ�Ĵ�
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

// ������
VOID __cdecl main(int argc, char* argv[])
{
    printf("---------------------------------------------------------------------------------------------------\n");
    printf(" <<%s %x.%x_%s, %s>>\n", VERSION_HANDLE, VERSION_H, VERSION_L, __DATE__, __TIME__);
    printf("---------------------------------------------------------------------------------------------------\n");

    // �������
    if (argc < 2)
    {
        printf("Usage:  %s <drive:>\n", argv[0]);
        return;
    }

    //// ��---------------------------------------------------------Create FileHandle ---------------------------------------------------------------��
    DWORD accessMode = 0; // ����ģʽ
    DWORD shareMode = 0; // ����ģʽ
    DWORD ReturnBytes = 0; // ���ص��ֽ���
    DWORD errorCode = 0; // �������
    HANDLE fileHandle = NULL; //�ļ����
    UCHAR string_path[25]; // �洢������·��
    UCHAR ubDataBuf[512] = { 0 }; // ���ݻ�����


    strncpy_s((char*)string_path, 25, "\\\\.\\", 24);
    strncat_s((char*)string_path, 25, argv[1], 24);

    printf("Current Drive: %s\n", string_path);
    printf("\n");

    // �����ļ����ʺ͹���ģʽ
    shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    accessMode = GENERIC_WRITE | GENERIC_READ;


    // ��������
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

    // ������
    DeviceIoControl(fileHandle,
        FSCTL_LOCK_VOLUME,
        NULL, 0,
        NULL, 0,
        &ReturnBytes,
        false);

    std::string searchString = "test"; //����Ѱ�Ұ������ַ������ļ���Ŀ¼
    std::string delete_file_path = "E:/TEST/TEST.TXT"; // Ҫɾ�����ļ����ļ�����

    std::string create_file_path = "E:"; // Ҫ�������ļ�·��

    std::string source_file_path = "E:/TEST/TEST.TXT"; // Ҫ���Ƶ�Դ�ļ�·��
    std::string destination_file_path = "E:"; // Ҫ���Ƶ�Ŀ��·��

    std::string recover_file_path = "E:/TEST1/HAHA1.JPG"; // Ҫ�ָ����ļ�·��

    // ��-----------------------------------------------------------Parsing FAT32-------------------------------------------------------------------��
    FAT32 fat32(fileHandle,L"E:"); // FAT32�Ľṹ��

    //// ��---------------------------------------------------------Serach file or dir---------------------------------------------------------------��
    //fat32.SearchInPathVector(searchString);

    //// ��---------------------------------------------------------Delete file or dir---------------------------------------------------------------��
    //fat32.DeleteSpecifiedFile(fileHandle, delete_file_path);

    //// ��---------------------------------------------------------Create file or dir---------------------------------------------------------------��
    /*fat32.CreateFileOrDir(fileHandle, create_file_path);*/

    //// ��---------------------------------------------------------Copy file or dir-----------------------------------------------------------------��
    // fat32.CopyFileOrDir(fileHandle, source_file_path, destination_file_path);

    //// ��---------------------------------------------------------Recover file-----------------------------------------------------------------��
    // fat32.RecoverFile(fileHandle, recover_file_path);

    // ������
    DeviceIoControl(fileHandle,
        FSCTL_UNLOCK_VOLUME,
        NULL, 0,
        NULL, 0,
        &ReturnBytes,
        false);

    // �ر��ļ����
    CloseHandle(fileHandle);

    system("pause"); // ��ͣ�Բ鿴���

    return;
}
