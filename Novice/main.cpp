#include <Novice.h>
#include <math.h>
#include <cassert>

const char kWindowTitle[] = "GC2B_03_ニャン_トー_セッ";

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Matrix4x4 {
	float m[4][4];
};

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result = {};
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;
	return result;
}

Vector3 Transform(const Vector3& position, const Matrix4x4& matrix) {
	Vector3 result = {};
	result.x = position.x * matrix.m[0][0] + position.y * matrix.m[1][0] + position.z * matrix.m[2][0] + matrix.m[3][0];
	result.y = position.x * matrix.m[0][1] + position.y * matrix.m[1][1] + position.z * matrix.m[2][1] + matrix.m[3][1];
	result.z = position.x * matrix.m[0][2] + position.y * matrix.m[1][2] + position.z * matrix.m[2][2] + matrix.m[3][2];
	float w = position.x * matrix.m[0][3] + position.y * matrix.m[1][3] + position.z * matrix.m[2][3] + matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;

	return result;
}

static const int kRowHeight = 20;
static const int kColumnWidth = 60;

void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {
	Novice::ScreenPrintf(x, y, "%0.2f", vector.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%0.2f", vector.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%0.2f", vector.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}


void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(x + column * kColumnWidth, y + (row + 1) * kRowHeight, "%6.02f", matrix.m[row][column]);
		}
	}
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 translate = {4.1f, 2.6f, 0.8f};
	Vector3 scale = {1.5f, 5.2f, 7.3f};

	Vector3 point = {2.3f, 3.8f, 1.4f};

	Matrix4x4 transformMatrix = {
		1.0f , 2.0f , 3.0f , 4.0f ,
		3.0f , 1.0f , 1.0f , 2.0f ,
		1.0f , 4.0f , 2.0f , 3.0f ,
		2.0f , 2.0f , 1.0f , 3.0f ,
    };

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
		
		
	    Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
		Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
		Vector3 transformed = Transform(point, transformMatrix);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		VectorScreenPrintf(0, 0, transformed, "transformed");
		MatrixScreenPrintf(0, kRowHeight , translateMatrix, "translateMatrix");
		MatrixScreenPrintf(0, kRowHeight * 6 , scaleMatrix, "scaleMatrix");

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
