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

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {

	Matrix4x4 matrix{};

	float cosX = cosf(rotate.x);
	float sinX = sinf(rotate.x);

	float cosY = cosf(rotate.y);
	float sinY = sinf(rotate.y);

	float cosZ = cosf(rotate.z);
	float sinZ = sinf(rotate.z);

	matrix.m[0][0] = scale.x * (cosY * cosZ);
	matrix.m[0][1] = scale.x * (cosY * sinZ);
	matrix.m[0][2] = scale.x * (-sinY);
	matrix.m[0][3] = 0.0f;

	matrix.m[1][0] = scale.y * (sinX * sinY * cosZ - cosX * sinZ);
	matrix.m[1][1] = scale.y * (sinX * sinY * sinZ + cosX * cosZ);
	matrix.m[1][2] = scale.y * (sinX * cosY);
	matrix.m[1][3] = 0.0f;

	matrix.m[2][0] = scale.z * (cosX * sinY * cosZ + sinX * sinZ);
	matrix.m[2][1] = scale.z * (cosX * sinY * sinZ - sinX * cosZ);
	matrix.m[2][2] = scale.z * (cosX * cosY);
	matrix.m[2][3] = 0.0f;

	matrix.m[3][0] = translate.x;
	matrix.m[3][1] = translate.y;
	matrix.m[3][2] = translate.z;
	matrix.m[3][3] = 1.0f;

	return matrix;
}

void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int i = 0; i < 4; ++i) {
		Novice::ScreenPrintf(x, y + (i + 1) * 20, "%6.2f %6.2f %6.2f %6.2f",
							  matrix.m[i][0], matrix.m[i][1], matrix.m[i][2], matrix.m[i][3]);
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 scale{1.2f, 0.79f, -2.1f};
	Vector3 rotate{0.4f, 1.43f, -0.8f};
	Vector3 translate{2.7f, -4.5f, 1.57f};


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
		Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		MatrixScreenPrintf(0, 0, worldMatrix, "worldMatrix");
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
