LRESULT CALLBACK DebugProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam){

	                        
    PWNDDATA pwd = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

	switch (message) {

        case WM_CREATE:
             
            return 0;
                     
        case WM_PAINT:
            DebugPaint(hWnd);
            return 0;

        case WM_COMMAND:
            
            return 0;

        case WM_DESTROY:
            break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
    }


}


VOID DebugPaint(HWND hWnd){
		                        
    PWNDDATA		pwd			= {0};
	HDC             hdc         = {0};
    PAINTSTRUCT     ps          = {0};
    

	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
	hdc = BeginPaint(hWnd, &ps) ;




	
    EndPaint(hWnd, &ps);
}
