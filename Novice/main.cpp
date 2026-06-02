#include <Novice.h>
#include <cstring>
#include <math.h>
const char kWindowTitle[] = "GC2B_03_ニャン_トー_セッ";

typedef struct Vector3 {

	float x;
	float y;
	float z;

} Vector3;

typedef struct Matrix4x4 {

	float m[4][4];

} Matrix4x4;

Matrix4x4 OrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {};

	float width = right - left;
	float height = top - bottom;
	float depth = farClip - nearClip;

	result.m[0][0] = 2.0f / width;
	result.m[1][1] = 2.0f / height;
	result.m[2][2] = 1.0f / depth;

	result.m[3][0] = -(right + left) / width;
	result.m[3][1] = -(top + bottom) / height;
	result.m[3][2] = -nearClip / depth;

	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 PerspectiveMatrix(float fovY, float aspect, float nearClip, float farClip) {
	Matrix4x4 result = {};
	float f = 1.0f / tanf(fovY / 2.0f);
	result.m[0][0] = f / aspect;
	result.m[1][1] = f;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -(farClip * nearClip) / (farClip - nearClip);
	return result;
}
Matrix4x4 MakeViewPortMatrix(float x, float y, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = {};

	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDepth - minDepth;

	result.m[3][0] = x + width / 2.0f;
	result.m[3][1] = y + height / 2.0f;
	result.m[3][2] = minDepth;

	result.m[3][3] = 1.0f;

	return result;
}
static const int kRowHeight = 20;
static const int kColumnWidth = 60;
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {

			Novice::ScreenPrintf(x + j * kColumnWidth, y + i * kRowHeight + 20, "%6.02f", matrix.m[i][j]);
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
		Matrix4x4 orthographicMatrix = OrthographicMatrix(-160.f, 160.f, 200.0f, 300.0f, 0.0f, 1000.0f);
		Matrix4x4 perspectiveFovMatrix = PerspectiveMatrix(0.63f, 1.33f, 0.1f, 1000.0f);
		Matrix4x4 viewportMatrix = MakeViewPortMatrix(100.0f, 200.0f, 600.0f, 300.0f, 0.0f, 1.0f);
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		MatrixScreenPrintf(0, 0, orthographicMatrix, "orthographicMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5, perspectiveFovMatrix, "perspectiveFovMatrix");
		MatrixScreenPrintf(0, kRowHeight * 10, viewportMatrix, "viewportMatrix");
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