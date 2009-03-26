class IEditor
{
public:
	void Update(){RedrawViews();}
	virtual void RedrawViews ()=0;
	virtual vector<EditorViewWindow *> GetViews () = 0;
	virtual void MergeView(EditorViewWindow *own, EditorViewWindow *other)=0;
	virtual void AddView(EditorViewWindow *v)=0;
	virtual void SelectionUpdated()=0;
	virtual Model* GetMdl()=0;
	virtual Tool* GetTool()=0;
	virtual void RenderScene(IView *view)=0;
	virtual TextureHandler* GetTextureHandler()=0;
	virtual void SetTextureSelectCallback(void (*cb)(Texture *tex, void *data), void *data)=0;
	virtual float GetTime()=0;
	virtual void SetModel(Model* mdl) =0;
};
