#include <Editor/Editor.h>

int main()
{
    grom::Editor editor;
    if (!editor.Initialize())
        return 1;
    editor.Run();
    editor.Shutdown();
    return 0;
}
