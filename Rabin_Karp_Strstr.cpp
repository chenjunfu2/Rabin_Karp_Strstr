#include <stdint.h>

//注意此函数默认子串长度至少小于等于查找字符串
uint64_t Rabin_Karp_Strstr(const char *pFindStr, const uint64_t u64FindStrLength, const char *pChildStr, const uint64_t u64ChildStrLength, const uint64_t u64HashMod = 255)
{
	if (u64FindStrLength < u64ChildStrLength)
	{
		return (uint64_t)-1;
	}

	auto Base256MoveLeft1 = [](uint64_t u64Val)->uint64_t
	{
		return u64Val * 256ui64;//0~255
	};

	//算出最大相乘的hash
	uint64_t u64MaxHash = 1;
	for (uint64_t i = 0; i < u64ChildStrLength; ++i)
	{
		u64MaxHash = Base256MoveLeft1(u64MaxHash);//左移
		u64MaxHash %= u64HashMod;//算hash
	}

	//首先按子串长度求出两个hash
	uint64_t u64FindStrHash = 0;
	uint64_t u64ChildStrHash = 0;

	for (uint64_t i = 0; i < u64ChildStrLength; ++i)
	{
		u64FindStrHash = (Base256MoveLeft1(u64FindStrHash) + pFindStr[i]);//挨个左移并或入位
		u64FindStrHash %= u64HashMod;//算hash
		u64ChildStrHash = (Base256MoveLeft1(u64ChildStrHash) + pChildStr[i]);//挨个左移并或入位
		u64ChildStrHash %= u64HashMod;//算hash
	}

	for (uint64_t i = 0; i <= u64FindStrLength - u64ChildStrLength; ++i)
	{
		if (u64FindStrHash != u64ChildStrHash)//大概率不匹配，条件写在前优先命中减少损耗
		{
			//不匹配滚动到下一个
			u64FindStrHash = u64FindStrHash - (pFindStr[i] * u64MaxHash) + u64HashMod;//删除高位,加上hash值防止数值过低导致的环绕
			u64FindStrHash = Base256MoveLeft1(u64FindStrHash) + pFindStr[i + u64ChildStrLength];//左移或入低位

			u64FindStrHash %= u64HashMod;//算hash
		}
		else//hash匹配，验证是否正确
		{
			for (uint64_t j = 0; j < u64ChildStrLength; ++j)
			{
				if (pFindStr[i + j] != pChildStr[j])
				{
					goto continue_next;
				}
			}
			//for能执行到这说明完全匹配，已经找到子串，直接返回
			return i;
		continue_next://否则继续循环
			continue;
		}
	
	}

	return (uint64_t)-1;
}


#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

bool GetFileData(const char *pFileName, char **ppData, uint64_t *pu64DataSize)
{
	FILE *fr = fopen(pFileName, "rb");
	if (fr == NULL)
	{
		return false;
	}

	fseek(fr, 0, SEEK_END);//定位到文件尾部
	uint64_t u64FileLong = ftell(fr);//获取文件大小
	rewind(fr);//回到文件开头

	char *pT = (char *)malloc(u64FileLong * sizeof(*pT));
	if (pT == NULL)
	{
		fclose(fr);
		return false;
	}

	if (fread(pT, sizeof(*pT), u64FileLong, fr) != u64FileLong)
	{
		fclose(fr);
		free(pT);
		return false;
	}

	fclose(fr);

	*ppData = pT;
	*pu64DataSize = u64FileLong;
	return true;
}

int main(int argc, char *argv[])
{
	if (argc != 4 && argc != 5)//0：自身路径 1：目标文件路径 2：查找字符串文件路径 3：输出文件路径 4：哈希求模值（可选）
	{
		printf("-1");
		return -1;
	}

	char *pFindData = NULL;
	uint64_t u64FindSize = NULL;
	if (!GetFileData(argv[1], &pFindData, &u64FindSize))
	{
		printf("-2");
		return -2;
	}

	char *pChildData = NULL;
	uint64_t u64ChildSize = NULL;
	if (!GetFileData(argv[2], &pChildData, &u64ChildSize))
	{
		printf("-3");
		return -3;
	}

	FILE *fw = fopen(argv[3], "wb");
	if (fw == NULL)
	{
		printf("-4");
		return -4;
	}

	uint64_t u64HashMod = 255;
	if (argc == 5)
	{
		if (sscanf(argv[4], "%lld", &u64HashMod) != 1)
		{
			printf("-5");
			return -5;
		}
	}

	fprintf(fw, "start in [%ld]\n\n", clock());

	uint64_t u64Ret;
	uint64_t u64Cur = 0;
	uint64_t i = 0;
	while ((u64Ret = Rabin_Karp_Strstr(&pFindData[u64Cur], u64FindSize - u64Cur, pChildData, u64ChildSize, u64HashMod)) != (uint64_t)-1)
	{
		fprintf(fw, "[%lld]|T[%lld]:{%.16s}\n", ++i, u64Cur + u64Ret, &pFindData[u64Cur + u64Ret]);
		u64Cur += u64Ret + u64ChildSize;
	}

	fprintf(fw, "\nend in [%ld]\nall:[%lld]\n", clock(), i);

	//清理资源
	fclose(fw), fw = NULL;
	free(pFindData), pFindData = NULL;
	free(pChildData), pChildData = NULL;

	return 0;
}