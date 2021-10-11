#include <iostream>
#include <Windows.h>
#include <mutex>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

std::mutex m;
std::atomic<bool> active = false;
bool key_and_mouse = false;
bool mouse_mode = false;
bool key_mode = false;
bool low_level = false;
int key = NULL;
bool pos = false;
bool _template = false;
std::atomic<bool> on_screen = false;
cv::Mat template_img;
std::atomic<int> temp_x;
std::atomic<int> temp_y;
cv::Mat screenshot_img;
std::string mode_message;

void screenshot(bool loop)
{
    while (1)
    {
        HWND hwnd = GetDesktopWindow();
        HDC hwindowDC, hwindowCompatibleDC;
        int height, width, srcheight, srcwidth;
        HBITMAP hbwindow;
        cv::Mat src;
        cv::Mat output;
        BITMAPINFOHEADER bi;
        hwindowDC = GetDC(hwnd);
        hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
        SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);
        RECT windowsize;
        GetClientRect(hwnd, &windowsize);
        srcheight = windowsize.bottom;
        srcwidth = windowsize.right;
        height = windowsize.bottom;
        width = windowsize.right;
        src.create(height, width, CV_8UC4);
        hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;
        SelectObject(hwindowCompatibleDC, hbwindow);
        StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY);
        GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        DeleteObject(hbwindow); DeleteDC(hwindowCompatibleDC); ReleaseDC(hwnd, hwindowDC);
        cv::resize(src, output, cv::Size(src.cols / 2, src.rows / 2), cv::INTER_LINEAR);
        if (loop)
        {
            m.lock();
            screenshot_img = output;
            m.unlock();
        }
        else
        {
            screenshot_img = output;
            break;
        }
    }
}

void template_pos(bool loop)
{
    while (1)
    {
        m.lock();
        cv::Mat src = screenshot_img.clone();
        m.unlock();
        cv::Mat result;
        int result_cols = src.cols - template_img.cols + 1;
        int result_rows = src.rows - template_img.rows + 1;
        result.create(result_rows, result_cols, CV_32FC1);
        cv::cvtColor(src, src, cv::COLOR_RGB2GRAY);
        matchTemplate(src, template_img, result, cv::TM_CCOEFF_NORMED);
        cv::threshold(result, result, 0.1, 1., cv::THRESH_TOZERO);
        double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
        cv::Point matchLoc;
        minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
        if (maxVal >= 0.8) {
            temp_x = (maxLoc.x * 2) + (template_img.cols / 2);
            temp_y = (maxLoc.y * 2) + (template_img.rows / 2);
            m.lock();
            on_screen = true;
            m.unlock();
        }
        else
        {
            m.lock();
            on_screen = false;
            m.unlock();
        }
    }
}

void load_template(std::string path)
{
    std::cout << path << std::endl;
    cv::Mat img = cv::imread(path);
    cv::cvtColor(img, img, cv::COLOR_RGB2GRAY);
    template_img = img;
}

void toggle()
{
    while (1)
    {
        if (GetAsyncKeyState(VK_F6) & 0x8000)
        {
            if (active == false)
            {
                active = true;
                std::cout << "enabled" << "\t\r" << std::flush;
                Sleep(200);
            }
            else
            {
                active = false;
                std::cout << "disabled" << "\t\r" << std::flush;
                Sleep(200);
            }
        }
        if (GetAsyncKeyState(VK_F7) & 0x8000)
        {
            exit(1);
        }
    }
}

int main(int argc, char* argv[])
{
    std::thread _screnshot;
    std::thread _template_pos;
    std::thread _toggle(toggle);
    int click_interval = 4;
    int x_pos, y_pos;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-t") == 0)
        {
            std::string template_path = argv[i + 1];
            load_template(template_path);
            screenshot(false);
            _screnshot = std::thread{ screenshot, true };
            _template_pos = std::thread{ template_pos , true };
            mouse_mode = true;
            _template = true;
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            x_pos = atoi(argv[i + 1]);
            y_pos = atoi(argv[i + 2]);
            mouse_mode = true;
            pos = true;
        }
        else if (strcmp(argv[i], "-k") == 0)
        {
            key_mode = true;
            key = strtoul(argv[i + 1], NULL, 16);
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            click_interval = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-km") == 0)
        {
            key_and_mouse = true;
            key = strtoul(argv[i + 1], NULL, 16);
        }
        else if (strcmp(argv[i], "-l") == 0)
        {
            low_level = true;
        }
    }
    if ((key_mode && mouse_mode) || (key_mode && key_and_mouse))
    {
        key_and_mouse = true;
        mouse_mode = false;
        key_mode = false;
    }
    else if (key_mode == false && mouse_mode == false && key_and_mouse == false)
    {
        mouse_mode = true;
    }
    else if ((key_mode || key_and_mouse) && (key == NULL))
    {
        std::cout << "keyboard was enabled, but no key was passed" << std::endl;
        exit(1);
    }

    if (key_and_mouse)
    {
        mode_message = mode_message + "keyboard mouse ";
    }
    else if (mouse_mode)
    {
        mode_message = mode_message + "mouse ";
    }
    else if (key_mode)
    {
        mode_message = mode_message + "keyboard ";
    }

    if (pos)
    {
        mode_message = mode_message + "pos ";
    }
    if (_template)
    {
        mode_message = mode_message + "template ";
    }
    if (low_level)
    {
        mode_message = mode_message + "low-level ";
    }
    std::cout << mode_message << "mode" << std::endl;

    while (1)
    {
        if (active)
        {
            if (_template)
            {
                if (on_screen)
                {
                    SetCursorPos(temp_x, temp_y);
                }
                else
                {
                    if (pos)
                    {
                        SetCursorPos(x_pos, y_pos);
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else if (pos)
            {
                SetCursorPos(x_pos, y_pos);
            }
       
            if (mouse_mode)
            {
                if (low_level)
                {
                    INPUT Inputs[1] = { 0 };
                    Inputs[0].type = INPUT_MOUSE;
                    Inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                    SendInput(1, Inputs, sizeof(INPUT));
                    Sleep(1);
                    ZeroMemory(&Inputs, sizeof(INPUT));
                    Inputs[0].type = INPUT_MOUSE;
                    Inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
                    SendInput(1, Inputs, sizeof(INPUT));
                }
                else
                {
                    mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                }

            }
            else if (key_mode)
            {
                keybd_event(key, 0, 0, 0);
                keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
            }
            else if (key_and_mouse)
            {
                keybd_event(key, 0, 0, 0);
                keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
                if (low_level)
                {
                    INPUT Inputs[1] = { 0 };
                    Inputs[0].type = INPUT_MOUSE;
                    Inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                    SendInput(1, Inputs, sizeof(INPUT));
                    Sleep(1);
                    ZeroMemory(&Inputs, sizeof(INPUT));
                    Inputs[0].type = INPUT_MOUSE;
                    Inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
                    SendInput(1, Inputs, sizeof(INPUT));
                }
                else
                {
                    mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                }
            }
            Sleep(click_interval);
        }
    }
    return 0;
}
