//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

	IEditor *callback;
	Timer *timer;

	float time;
	unsigned int prevTicks;
	bool isPlaying;

	void SliderCallback ();
	void cmdSetLength ();
	void cmdPlayStop ();
	void cmdPause ();

	void Play ();
	void Stop ();

	static void idle_cb (void *arg);

public:
	TimelineUI (IEditor *editor);
	~TimelineUI ();

	float GetTime () { return time; }
	void Show ();
	void Hide ();
	void Update ();
	void InsertKeys (bool autoInsert=false);
