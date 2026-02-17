#pragma once
#include <memory>

class hoge {
private:
	int a;		

public:
	~hoge() {
		// デストラクタ;
	}
};

int main() {

	// unique_ptrの宣言
	std::unique_ptr<hoge> p0;

	// hoge型のunique_ptrを生成して代入
	p0 = std::make_unique<hoge>();

	// p0の解放と同時にひそかにdeleteが呼ばれ、p0のさすリソースが解放される
	return 0;

}

