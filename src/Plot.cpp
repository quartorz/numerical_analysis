#include "4.h"
#include <concurrent_queue.h>
#include <vector>
#include <algorithm>
#include <process.h>
#include <dwrite.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#define CLASSNAME L"NumericalAnalysis_PlotWindow"

namespace{
	HINSTANCE hinst = ::GetModuleHandleW(NULL);
	HANDLE hMsg;
	struct Message{
		enum{
			CREATE,
			DESTROY,
			END
		}msg;
		void *data;
		void *ret;
	};
	ID2D1Factory *factory;
	IDWriteFactory *dwfactory;
	IDWriteTextFormat *font;
	Concurrency::concurrent_queue<Message*> queue;
	bool end = false;
	int count = 0;
	unsigned int __stdcall MessageLoop(void *)
	{
		if(FAILED(::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
			return 0;
		if(FAILED(::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &factory)))
			return 0;
		if(FAILED(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwfactory), reinterpret_cast<IUnknown**>(&dwfactory))))
			return 0;
		if(FAILED(dwfactory->CreateTextFormat(
			L"Meiryo",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			18.f,
			L"ja-JP",
			&font
		)))
			return 0;

		::SetEvent(hMsg);

		BOOL ret;
		MSG msg;
		do{
			if(::PeekMessageW(&msg,NULL,0,0,PM_NOREMOVE)){
				ret = ::GetMessageW(&msg,NULL,0,0);
				if(ret == 0 || ret == -1)
					break;
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}else{
				if(queue.empty())
					continue;
				Message *d;
				if(!queue.try_pop(d))
					continue;
				switch(d->msg){
				case Message::CREATE:
					d->ret = ::CreateWindowExW(
						0,
						CLASSNAME,
						L"",
						WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, 0,
						CW_USEDEFAULT, 0,
						NULL,
						NULL,
						::GetModuleHandleW(NULL),
						d->data
						);
					if(d->ret == NULL){
						wchar_t a[20];
						::wsprintfW(a, L"%X", ::GetLastError());
						::MessageBoxW(0, a, 0, 0);
					}
					::ShowWindow(static_cast<HWND>(d->ret), SW_SHOW);
					::UpdateWindow(static_cast<HWND>(d->ret));
					count++;
					break;
				case Message::DESTROY:
					::DestroyWindow(static_cast<HWND>(d->data));
					count--;
					if(end && count == 0)
						goto BREAK;
					break;
				case Message::END:
					end = true;
					if(count == 0)
						goto BREAK;
					break;
				default:
					__assume(0);
				}
				::SetEvent(hMsg);
			}
		}while(msg.message != WM_QUIT);
BREAK:;
		factory->Release();
		dwfactory->Release();
		font->Release();
		::SetEvent(hMsg);
		return 0;
	}
	void *Send(Message *m)
	{
		::ResetEvent(hMsg);
		queue.push(m);
		::WaitForSingleObject(hMsg, INFINITE);
		return m->ret;
	}

	inline D2D1_RECT_F operator+(const D2D1_RECT_F &r, const D2D1_POINT_2F &p)
	{
		return D2D1::RectF(
			r.left + p.x,
			r.top + p.y,
			r.right + p.x,
			r.bottom + p.y
			);
	}

	inline D2D1_RECT_F operator+(const D2D1_POINT_2F &p, const D2D1_RECT_F &r)
	{
		return r + p;
	}
}

namespace numerical_analysis{
	bool plot_init()
	{
		WNDCLASSEX wcex;
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= Plot::WindowProc_Static;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= sizeof(LONG_PTR);
		wcex.hInstance		= hinst;
		wcex.hIcon			= NULL; 
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= NULL;
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= CLASSNAME;
		wcex.hIconSm		= NULL;

		if(::RegisterClassExW(&wcex) == NULL)
			return false;

		hMsg = ::CreateEventW(NULL, FALSE, FALSE, NULL);
		if(hMsg == NULL)
			return false;

		::ResetEvent(hMsg);

		if(::_beginthreadex(NULL, 0, MessageLoop, NULL, 0, NULL) == 0)
			return false;

		if(::WaitForSingleObject(hMsg, 3000) != WAIT_OBJECT_0)
			return false;

		return true;
	}
	void plot_uninit()
	{
		Message m = {Message::END};
		Send(&m);
		::CloseHandle(hMsg);
	}

	struct SubplotData{
		typedef std::pair<double, double> point_type;
		typedef std::vector<point_type> line_type;
		std::vector<line_type> lines;
		std::vector<point_type> points;
		std::wstring title;
		IDWriteTextLayout *dtitle;
		DWRITE_TEXT_ALIGNMENT xalign;
		DWRITE_PARAGRAPH_ALIGNMENT yalign;
		double xrange[2], yrange[2];
		D2D1_COLOR_F color;
		ID2D1SolidColorBrush *br, *defb[4];
		static const D2D1_COLOR_F defc[4];
		SubplotData(): dtitle(NULL), color(D2D1::ColorF(D2D1::ColorF::Black)), br(NULL), defb()
		{
			Reset();
		}
		void Reset()
		{
			lines.clear();
			title.clear();
			if(dtitle != NULL){
				dtitle->Release();
				dtitle = NULL;
			}
			xalign = DWRITE_TEXT_ALIGNMENT_CENTER;
			yalign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
			xrange[0] = -1;
			xrange[1] = 1;
			yrange[0] = -1;
			yrange[1] = 1;
			color = D2D1::ColorF(D2D1::ColorF::Black);
			D2DDestroy();
		}
		bool D2DCreate(ID2D1RenderTarget *t)
		{
			if(br == NULL)
				if(FAILED(t->CreateSolidColorBrush(color, &br)))
					return D2DDestroy(), false;
			for(int i = 0; i < _countof(defb); i++)
				if(defb[i] == NULL)
					if(FAILED(t->CreateSolidColorBrush(defc[i], &defb[i])))
						return D2DDestroy(), false;
			return true;
		}
		void D2DDestroy()
		{
			if(br != NULL){
				br->Release();
				br = NULL;
			}
			std::for_each(defb, defb + _countof(defb), [](ID2D1SolidColorBrush *br){
				if(br != NULL){
					br->Release();
					br = NULL;
				}
			});
		}
	};
	const D2D1_COLOR_F SubplotData::defc[4] = {
		D2D1::ColorF(D2D1::ColorF::Brown),
		D2D1::ColorF(D2D1::ColorF::Blue),
		D2D1::ColorF(D2D1::ColorF::Green),
		D2D1::ColorF(D2D1::ColorF::Orange),
	};

	LRESULT CALLBACK Plot::WindowProc_Static(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if(msg == WM_NCCREATE){
			auto lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lpcs->lpCreateParams));
		}
		auto plt = reinterpret_cast<Plot*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if(plt == NULL)
			return ::DefWindowProcW(hwnd, msg, wParam, lParam);
		return plt->WindowProc(hwnd, msg, wParam, lParam);
	}
	bool Plot::CreateResource()
	{
		if(target == NULL){
			RECT rc;
			::GetClientRect(hwnd, &rc);
			if(FAILED(
				factory->CreateHwndRenderTarget(
					D2D1::RenderTargetProperties(),
					D2D1::HwndRenderTargetProperties(hwnd,D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top)),
					&target
				)
			))
				return false;
		}
		for(auto it = plotdata.begin(); it != plotdata.end(); ++it)
			if(!it->D2DCreate(target))
				return DestroyResource(), false;
		return true;

	}
	void Plot::DestroyResource()
	{
		for(auto it = plotdata.begin(); it != plotdata.end(); ++it)
			it->D2DDestroy();
		if(target != NULL){
			target->Release();
			target = NULL;
		}
	}
	void Plot::OnPaint()
	{
		PAINTSTRUCT ps;
		::BeginPaint(hwnd, &ps);
		::EndPaint(hwnd, &ps);

		if(CreateResource()){
			target->BeginDraw();
			target->Clear(D2D1::ColorF(D2D1::ColorF::White));

			RECT rc;
			::GetClientRect(hwnd, &rc);

			auto clip = D2D1::RectF(0, 0, (FLOAT)rc.right / column, (FLOAT)rc.bottom / row);
			int plt_n = 0;
			for(int i = 0; i < row; i++){
				auto clip = D2D1::RectF(0, 0, (FLOAT)rc.right / column, (FLOAT)rc.bottom / row);
				
				for(int j = 0; j < column; j++, plt_n++){
					auto trans_subplot = D2D1::Point2F((FLOAT)rc.right * j / column, (FLOAT)rc.bottom * i / row);
					target->SetTransform(D2D1::Matrix3x2F::Translation(trans_subplot.x, trans_subplot.y));
					target->PushAxisAlignedClip(clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
//					target->FillRectangle(D2D1::RectF(0, 0, rc.right, rc.bottom), plotdata[sub].defb[i % 4]);
					auto &data = plotdata[plt_n];
					auto title = data.dtitle;
					if(title != NULL){
						title->SetMaxWidth(clip.right);
						title->SetMaxHeight(clip.bottom);
						target->DrawTextLayout(D2D1::Point2F(), title, plotdata[0].br);
					}
					FLOAT x_ratio = clip.right / (data.xrange[1] - data.xrange[0]);
					FLOAT y_ratio = clip.bottom / (data.yrange[1] - data.yrange[0]);

					auto origin = D2D1::Point2F(- data.xrange[0] * x_ratio, data.yrange[1] * y_ratio);

					{
						// draw axis
						auto origin_axis = origin;
						bool x_left = false, y_bottom = false;
						if(origin.x < 20)
							x_left = true;
						if(origin.x < 0)
							origin_axis.x = 0;
						else if(origin.x > clip.right)
							origin_axis.x = clip.right;
						target->DrawLine(
							D2D1::Point2F(origin_axis.x, 0),
							D2D1::Point2F(origin_axis.x, clip.bottom),
							data.br
							);
						target->DrawLine(
							D2D1::Point2F(origin_axis.x, 0),
							D2D1::Point2F(origin_axis.x + 5, 15),
							data.br
							);
						target->DrawLine(
							D2D1::Point2F(origin_axis.x, 0),
							D2D1::Point2F(origin_axis.x - 5, 15),
							data.br
							);
						if(origin.y > clip.bottom - 20)
							y_bottom = true;
						if(origin.y < 0)
							origin_axis.y = 0;
						else if(origin.y > clip.bottom)
							origin_axis.y = clip.bottom;
						target->DrawLine(
							D2D1::Point2F(0, origin_axis.y),
							D2D1::Point2F(clip.right, origin_axis.y),
							data.br
							);
						target->DrawLine(
							D2D1::Point2F(clip.right, origin_axis.y),
							D2D1::Point2F(clip.right - 15, origin_axis.y + 5),
							data.br
							);
						target->DrawLine(
							D2D1::Point2F(clip.right, origin_axis.y),
							D2D1::Point2F(clip.right - 15, origin_axis.y - 5),
							data.br
							);
						data.br->SetOpacity(.5f);
						for(int i = std::floor(data.xrange[0]), i_max = std::ceil(data.xrange[1]); i <= i_max; i++){
							target->DrawLine(
								D2D1::Point2F(i * x_ratio + origin.x, origin_axis.y - 5),
								D2D1::Point2F(i * x_ratio + origin.x, origin_axis.y + 5),
								data.br
								);
							wchar_t x[10];
							::wsprintfW(x, L"%d", i);
							target->DrawTextW(
								x,
								::wcslen(x),
								font,
								D2D1::RectF(i * x_ratio + origin.x - 10, origin_axis.y + (y_bottom ? -25 : 0), D2D1::FloatMax(), D2D1::FloatMax()),
								data.br
								);
						}
						for(int i = std::floor(data.yrange[0]), i_max = std::ceil(data.yrange[1]); i <= i_max; i++){
							target->DrawLine(
								D2D1::Point2F(origin_axis.x - 5, - i * y_ratio + origin.y),
								D2D1::Point2F(origin_axis.x + 5, - i * y_ratio + origin.y),
								data.br
								);
							wchar_t y[10];
							::wsprintfW(y, L"%d", i);
							target->DrawTextW(
								y,
								::wcslen(y),
								font,
								D2D1::RectF(origin_axis.x - 30 + (x_left ? +35 : 0), - i * y_ratio + origin.y - 10, D2D1::FloatMax(), D2D1::FloatMax()),
								data.br
								);
						}
						data.br->SetOpacity(1.f);
					}

					// draw graphs
					int k = 0;
					for(auto it_ls = data.lines.begin(); it_ls != data.lines.end(); ++it_ls){
						for(auto it_l = it_ls->begin() + 1; it_l != it_ls->end(); ++it_l){
							target->DrawLine(
								D2D1::Point2F(it_l[-1].first * x_ratio + origin.x, - it_l[-1].second * y_ratio + origin.y),
								D2D1::Point2F(it_l->first * x_ratio + origin.x, - it_l->second * y_ratio + origin.y),
								data.defb[k % 4]
								);
						}
						k++;
					}

					// draw points
					for(auto it = data.points.begin(); it != data.points.end(); ++it){
						target->FillEllipse(
							D2D1::Ellipse(D2D1::Point2F(it->first * x_ratio + origin.x, - it->second * y_ratio + origin.y), 3, 3),
							data.br
							);
					}

					target->PopAxisAlignedClip();
				}
			}

			target->SetTransform(D2D1::Matrix3x2F::Identity());

			for(int i = 2; i <= column; i++){
				target->DrawLine(D2D1::Point2F((FLOAT)rc.right / i), D2D1::Point2F((FLOAT)rc.right / i, rc.bottom), plotdata[0].br, 3);
			}
			for(int i = 2; i <= row; i++){
				target->DrawLine(D2D1::Point2F(0, (FLOAT)rc.bottom / i), D2D1::Point2F(rc.right, (FLOAT)rc.bottom / i), plotdata[0].br, 3);
			}

			HRESULT hr = target->EndDraw();
			if(hr == D2DERR_RECREATE_TARGET){
				DestroyResource();
				::InvalidateRect(hwnd, NULL, FALSE);
			}
		}
	}
	LRESULT CALLBACK Plot::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg){
		case WM_CREATE:
			this->hwnd = hwnd;
			if(!CreateResource())
				return -1;
			create_subplot(1, 1);
			break;
		case WM_SIZE:
			target->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
			break;
		case WM_PAINT:
			::EnterCriticalSection(&cs);
			OnPaint();
			::LeaveCriticalSection(&cs);
			break;
		}
		return ::DefWindowProc(hwnd, msg, wParam, lParam);
	}


	Plot::Plot(const wchar_t *window_title)
		: hwnd(NULL), target(NULL), row(1), column(1), sub(0), mode(0)
	{
		::InitializeCriticalSection(&cs);
		Message msg = {Message::CREATE, this};
		hwnd = static_cast<HWND>(Send(&msg));
		set_window_title(window_title);

	}
	Plot::~Plot()
	{
		Message msg = {Message::DESTROY, hwnd};
		Send(&msg);
		::DeleteCriticalSection(&cs);
	}
	void Plot::set_window_title(const wchar_t *title)
	{
		::SetWindowTextW(hwnd, title);
	}
	void Plot::create_subplot(unsigned row, unsigned column)
	{
		if(row < 1 || column < 1)
			return;
		::EnterCriticalSection(&cs);
		for(int i = row * column; i < plotdata.size(); i++)
			plotdata[i].Reset();
		plotdata.resize(row * column);
		this->row = row;
		this->column = column;
		if(sub > row * column - 1){
			sub = row * column - 1;
		}
		CreateResource();
		::LeaveCriticalSection(&cs);
		::InvalidateRect(hwnd, NULL, TRUE);
	}
	void Plot::set_subplot(unsigned row, unsigned column)
	{
		::EnterCriticalSection(&cs);
		if(row >= this->row || column >= this->column)
			sub = this->row * this->column - 1;
		else
			sub = (row + 1) * (column + 1) - 1;
		::LeaveCriticalSection(&cs);
	}
	void Plot::set_subplot(unsigned subplot)
	{
		::EnterCriticalSection(&cs);
		if(subplot > row * column - 1)
			sub = row * column - 1;
		else
			sub = subplot;
		::LeaveCriticalSection(&cs);
	}
	void Plot::set_plot_mode(PlotMode mode)
	{
		this->mode = static_cast<int>(mode);
	}
	void Plot::plot(std::vector<double> &x, std::vector<double> &y)
	{
		::EnterCriticalSection(&cs);
		if(mode == 0){
			plotdata[sub].lines.push_back(SubplotData::line_type());
			auto it = plotdata[sub].lines.end() - 1;
			for(auto it1 = x.begin(), it2 = y.begin(); it1 != x.end() && it2 != y.end(); ++it1, ++it2)
				it->push_back(std::make_pair(*it1, *it2));
		}else{
			for(auto it1 = x.begin(), it2 = y.begin(); it1 != x.end() && it2 != y.end(); ++it1, ++it2)
				plotdata[sub].points.push_back(std::make_pair(*it1, *it2));
		}
		::LeaveCriticalSection(&cs);
		::InvalidateRect(hwnd, NULL, TRUE);
	}
	void Plot::plot(std::vector<std::pair<double, double>> &pt)
	{
		::EnterCriticalSection(&cs);
		if(mode == 0)
			plotdata[sub].lines.push_back(pt);
		else
			plotdata[sub].points.insert(plotdata[sub].points.end(), pt.begin(), pt.end());
		::LeaveCriticalSection(&cs);
		::InvalidateRect(hwnd, NULL, TRUE);
	}
	void Plot::set_plot_title(const wchar_t *title)
	{
		::EnterCriticalSection(&cs);
		plotdata[sub].title = title;
		if(plotdata[sub].dtitle != NULL)
			plotdata[sub].dtitle->Release();
		dwfactory->CreateTextLayout(
			title,
			::wcslen(title),
			font,
			D2D1::FloatMax(),
			D2D1::FloatMax(),
			&plotdata[sub].dtitle
			);
		plotdata[sub].dtitle->SetTextAlignment(plotdata[sub].xalign);
		plotdata[sub].dtitle->SetParagraphAlignment(plotdata[sub].yalign);
		::LeaveCriticalSection(&cs);
		::InvalidateRect(hwnd, NULL, TRUE);
	}
	void Plot::set_plot_title_pos(Position x, Position y)
	{
		switch(x){
		case Left:
			plotdata[sub].xalign = DWRITE_TEXT_ALIGNMENT_LEADING;
			break;
		case Center:
			plotdata[sub].xalign = DWRITE_TEXT_ALIGNMENT_CENTER;
			break;
		case Right:
			plotdata[sub].xalign = DWRITE_TEXT_ALIGNMENT_TRAILING;
			break;
		}
		if(plotdata[sub].dtitle != NULL)
			plotdata[sub].dtitle->SetTextAlignment(plotdata[sub].xalign);
		switch(y){
		case Top:
			plotdata[sub].yalign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
			break;
		case Center:
			plotdata[sub].yalign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
			break;
		case Right:
			plotdata[sub].yalign = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
			break;
		}
		if(plotdata[sub].dtitle != NULL)
			plotdata[sub].dtitle->SetParagraphAlignment(plotdata[sub].yalign);
	}
	void Plot::set_x_range(double min, double max)
	{
		::EnterCriticalSection(&cs);
		if(min == max)
			return;
		else if(min > max){
			plotdata[sub].xrange[0] = max;
			plotdata[sub].xrange[1] = min;
		}else{
			plotdata[sub].xrange[0] = min;
			plotdata[sub].xrange[1] = max;
		}
		::LeaveCriticalSection(&cs);
		::InvalidateRect(hwnd, NULL, TRUE);
	}
	void Plot::set_y_range(double min, double max)
	{
		::EnterCriticalSection(&cs);
		if(min == max)
			return;
		else if(min > max){
			plotdata[sub].yrange[0] = max;
			plotdata[sub].yrange[1] = min;
		}else{
			plotdata[sub].yrange[0] = min;
			plotdata[sub].yrange[1] = max;
		}
		::LeaveCriticalSection(&cs);
		::InvalidateRect(hwnd, NULL, TRUE);
	}
}
