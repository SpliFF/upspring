
public:
	TexBuilderUI(const char *tex1, const char *tex2);
	~TexBuilderUI ();

	void BuildTexture1();
	void BuildTexture2();
	void Browse(fltk::Input *inputBox, bool isOutput=false);
	void Show();
	unsigned int LoadImg (const char *fn);
