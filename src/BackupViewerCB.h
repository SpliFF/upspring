
BackupViewerUI(IEditor *callback);
~BackupViewerUI();

void SetBufferSize();
void Show();
void Hide();
void Update();
static void BufferViewerCallback(fltk::Widget *w, void *d);

IEditor *callback;
