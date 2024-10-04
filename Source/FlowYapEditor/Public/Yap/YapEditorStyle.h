#pragma once

class FYapEditorStyle final : public FSlateStyleSet
{
public:
	static FYapEditorStyle& Get()
	{
		static FYapEditorStyle Instance;
		return Instance;
	}

	
	FYapEditorStyle();

	virtual ~FYapEditorStyle() override;

	FDelegateHandle OnPatchCompleteHandle;

protected:
	void OnPatchComplete();

	void Initialize();

	FTextBlockStyle NormalText;
};