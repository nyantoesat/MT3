#include <Novice.h>
#include <cmath>

const char kWindowTitle[] = "GC2B_03_ニャン_トー_セッ";
typedef struct Vector3 {
	float x;
	float y;
	float z;
} Vector3;

Vector3 Add(const Vector3& v1, const Vector3& v2) {
Vector3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
};

Vector3 Subtract(const Vector3& v1, const Vector3& v2) { 
	Vector3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
};

Vector3 Multiply(const Vector3& v, float k) {
Vector3 result;
	result.x = v.x * k;
	result.y = v.y * k;
	result.z = v.z * k;
	return result;
};

float Dot(const Vector3& v1, const Vector3& v2) { 
	float result;
	result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return result;

};

float Length(const Vector3& v) {
float result;
	result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
return result;

};

Vector3 Normalize(const Vector3& v) {
Vector3 result;
	float length = Length(v);
	
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	
	return result;

};



static const int kColumnWidth = 60;
static const int kRowHeight = 20;

void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) { 
	Novice::ScreenPrintf(x, y, "%0.2f", vector.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%0.2f", vector.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%0.2f", vector.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	
	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 v1{1.0f, 3.0f, -5.0f};
	Vector3 v2{4.0f, -2.0f, 3.0f};
	float k = {4.0f};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

Vector3 resultAdd = Add(v1, v2);
		Vector3 resultSubtract = Subtract(v1, v2);
		Vector3 resultMultiply = Multiply(v1, k);
		float resultDot = Dot(v1, v2);
		float resultLength = Length(v1);
		Vector3 resultNormalize = Normalize(v2);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		VectorScreenPrintf(0, 0, resultAdd, "Add");
		VectorScreenPrintf(0, kRowHeight, resultSubtract, "Subtract");
		VectorScreenPrintf(0, kRowHeight * 2, resultMultiply, "Multiply");
		Novice::ScreenPrintf(0, kRowHeight * 3, "%0.2f : Dot", resultDot);
		Novice::ScreenPrintf(0, kRowHeight * 4, "%0.2f : Lenght", resultLength);
		VectorScreenPrintf(0, kRowHeight * 5, resultNormalize, "Normalize");

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
