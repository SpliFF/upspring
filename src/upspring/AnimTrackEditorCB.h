
	IEditor *callback;
	TimelineUI *timeline;

	friend class AnimKeyTrackView;

	struct BrowserItem {
		BrowserItem() { selected=open=false; }
		bool selected, open;
		virtual bool IsProperty() { return false; }
	};

	struct Property : BrowserItem {
		std::vector<bool> keySel;
		bool display;
		AnimProperty *prop;
		int member;
		bool IsProperty() { return true; }
		float EvaluateY (float time, int &lastkey);
	};
	struct AnimObject : BrowserItem {
		AnimObject(){obj=0;}
		MdlObject *obj;
		std::vector<Property> props;
	};
	std::list<AnimObject> objects;

	void AddObject (MdlObject *o);

	struct List : public fltk::List {
		List ();
		fltk::Widget *child (const fltk::Menu *, const int *indexes, int level);
		int children (const fltk::Menu *, const int *indexes, int level);
		void flags_changed (const fltk::Menu *, fltk::Widget *);
		static void item_callback (fltk::Widget *w, void *user_data);

		struct Item : fltk::Item {
			AnimTrackEditorUI* ui;
		} item;
		AnimTrackEditorUI* ui;
		string itemLabel;
	} browserList;

	void toggleMoveView();
	void cmdAutoFitView();
	void cmdDeleteKeys();
	void cmdAutoFitTime();

public:
	AnimTrackEditorUI(IEditor *editor, TimelineUI *timeline);
	~AnimTrackEditorUI();
	
	void Show ();
	void Hide ();
	void Update ();
	void UpdateBrowser ();
	void UpdateKeySel ();
