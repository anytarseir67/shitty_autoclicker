#include <iostream>
#include <Windows.h>
#include <mutex>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

std::mutex m;
std::atomic<bool> active = false;
bool key_mode = false;
int key;
bool pos = false;
bool _template = false;
std::atomic<bool> on_screen = false;
cv::Mat template_img;
std::atomic<int> temp_x;
std::atomic<int> temp_y;
cv::Mat screenshot_img;

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
        if (loop == true)
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
    int x_pos, y_pos;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-t") == 0)
        {
           std::string template_path = argv[i+1];
           load_template(template_path);
           screenshot(false);
           _screnshot = std::thread{ screenshot, true };
           _template_pos = std::thread{ template_pos , true };

           _template = true;
           std::cout << "template mode." << std::endl;
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            x_pos = atoi(argv[i+1]);
            y_pos = atoi(argv[i+2]);
            pos = true;
            std::cout << "position mode." << std::endl;
        }
        else if (strcmp(argv[i], "-k") == 0)
        {
            key_mode = true;
            key = strtoul(argv[i + 1], NULL, 16);
            std::cout << "key mode." << std::endl;
            break;
        }
    }
    while (1)
    {
        if (active == true)
        {
            if (_template == true)
            {
                if (on_screen == true)
                {
                    SetCursorPos(temp_x, temp_y);
                }
                else
                {
                    if (pos == true)
                    {
                        SetCursorPos(x_pos, y_pos);
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else if (pos == true)
            {
                SetCursorPos(x_pos, y_pos);
            }
       
            if (key_mode == true)
            {
                keybd_event(key, 0, 0, 0);
                keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
            }
            else
            {
                mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }
            Sleep(15);
        }
    }
    return 0;
}