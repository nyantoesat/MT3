#include <Novice.h>
#include <cmath>

const char kWindowTitle[] = "GC2B_03_ニャン_トー_セッ";

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Matrix4x4 {
	float m[4][4];
};

Matrix4x4 MakeRotateXMatrix(float radiam) {
	Matrix4x4 rotateXMatrix = {};
	rotateXMatrix.m[0][0] = 1.0f;
	rotateXMatrix.m[1][1] = std::cos(radiam);
	rotateXMatrix.m[1][2] = std::sin(radiam);
	rotateXMatrix.m[2][1] = -std::sin(radiam);
	rotateXMatrix.m[2][2] = std::cos(radiam);
	rotateXMatrix.m[3][3] = 1.0f;
	return rotateXMatrix;
}

Matrix4x4 MakeRotateYMatrix(float radiam) {
	Matrix4x4 rotateYMatrix = {};
	rotateYMatrix.m[0][0] = std::cos(radiam);
	rotateYMatrix.m[0][2] = -std::sin(radiam);
	rotateYMatrix.m[1][1] = 1.0f;
	rotateYMatrix.m[2][0] = std::sin(radiam);
	rotateYMatrix.m[2][2] = std::cos(radiam);
	rotateYMatrix.m[3][3] = 1.0f;
	return rotateYMatrix;
}

Matrix4x4 MakeRotateZMatrix(float radiam) {
	Matrix4x4 rotateZMatrix = {};
	rotateZMatrix.m[0][0] = std::cos(radiam);
	rotateZMatrix.m[0][1] = std::sin(radiam);
	rotateZMatrix.m[1][0] = -std::sin(radiam);
	rotateZMatrix.m[1][1] = std::cos(radiam);
	rotateZMatrix.m[2][2] = 1.0f;
	rotateZMatrix.m[3][3] = 1.0f;
	return rotateZMatrix;
}

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			for (int k = 0; k < 4; ++k) {
				result.m[row][column] += m1.m[row][k] * m2.m[k][column];
			}
		}
	}
	return result;
}

Matrix4x4 MakeRotateXYZMatrix(float xRadiam, float yRadiam, float zRadiam) {
	Matrix4x4 x = MakeRotateXMatrix(xRadiam);
	Matrix4x4 y = MakeRotateYMatrix(yRadiam);
	Matrix4x4 z = MakeRotateZMatrix(zRadiam);
	return Multiply(Multiply(x, y), z);
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

	Vector3 rotate = {0.4f, 1.43f, -0.8f};

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
		Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
		Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
		Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
		Matrix4x4 rotateXYZMatrix = MakeRotateXYZMatrix(rotate.x, rotate.y, rotate.z);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 0, rotateXMatrix, "rotateXMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5, rotateYMatrix, "rotateYMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5 * 2, rotateZMatrix, "rotateZMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5 * 3, rotateXYZMatrix, "rotateXYZMatrix");

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