#include "pl0.h"
#include <fstream>
#include <windows.h>

int main(int argc, char *argv[])
{
    char *fn;

    if (argc == 1)
    {
        // https://docs.microsoft.com/zh-cn/windows/desktop/dlgbox/using-common-dialog-boxes
        
        OPENFILENAMEA ofn;       // common dialog box structure
        char szFile[260];       // buffer for file name
        HWND hwnd = 0;              // owner window

        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szFile;

        // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
        // use the contents of szFile to initialize itself.
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All\0*.*\0pl0\0*.pl0\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        // Display the Open dialog box.
        if (GetOpenFileNameA(&ofn) == TRUE)
            fn = ofn.lpstrFile;
        else
            return 0;
    }
    else
    {
        fn = argv[1];
    }


    std::ifstream sf(fn);
    if (!sf.is_open()) return 1;

    sf.seekg(0, std::ios::end);    // go to the end
    unsigned int length = (unsigned int)sf.tellg();
    sf.seekg(0, std::ios::beg);    // go back to the beginning

    char *f = new char[length];    // allocate memory for a buffer of appropriate dimension  
    sf.read(f, length);       // read the whole file into the buffer  
    sf.close();                    // close file handle

    pl0_t test(f);
    if( test.succ )
        test.show_code();
    else
    {
        test.show_error();
        getchar();
        return 0;
    }

    int errorno, errorplace;
    if (!test.interpret(&errorno, &errorplace))
    {

    }

    getchar();
    return 0;
}