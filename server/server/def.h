#pragma once

// �̱��� ���� cpp ���� �����ڸ� �� �������־���Ѵ�

#define SINGLE(type)						\
public:										\
	static type* GetInstance()				\
	{										\
		static type* mInstance;				\
		if (mInstance == nullptr)			\
		{									\
			mInstance = new type();			\
			return mInstance;				\
		}									\
		return mInstance;					\
	}										\
											\
	static void DestroyInstance()			\
	{										\
	if (GetInstance())						\
		delete GetInstance();				\
	}										\
											\
private:									\
	type(); 								\
	~type(); 								\


// �̱��� �ν��Ͻ� ��ȯ
#define GETSINGLE(type)	\
type::GetInstance()		\

// ��ȯ��, Ÿ��, �����Ǵ� �޼ҵ��
#define GETSET(__TYPE__, __TARGET__, __METHODNAME__) \
__TYPE__ Get##__METHODNAME__()						 \
{													 \
	return __TARGET__;								 \
}													 \
													 \
void Set##__METHODNAME__(__TYPE__ value)			 \
{													 \
	__TARGET__ = value;								 \
}													 \