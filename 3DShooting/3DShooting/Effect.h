#pragma once

/// <summary>
/// �G�t�F�N�g�N���X
/// </summary>
class Effect
{
public:
	Effect();
	virtual ~Effect();

	void Init();
	void Update();
	void Draw();
	
	/// <summary>
	/// �}�Y���t���b�V�����Đ�����
	/// </summary>
	/// <param name="x">X���W</param>
	/// <param name="y">Y���W</param>
	/// <param name="z">Z���W</param>
	void PlayMuzzleFlash(float x, float y, float z, float rotX, float rotY, float rotZ);

private:
	int muzzleFlashEffectHandle; // �}�Y���t���b�V���̃G�t�F�N�g�n���h��
};

