//
// register application.
// Author:yeyouqun@163.com
// 2009-11-15
//

#ifndef __GM_APP_FRAME_H__
#define __GM_APP_FRAME_H__

class GmRegisterFrame : public wxFrame
{
public:
	/**
	 * ctor
	 */
	GmRegisterFrame (const wxString& title);
				
	/**
	 * dtor
	 */
	~GmRegisterFrame (void);


private:
	void OnPaint(wxPaintEvent &);
	void OnCloseRegApp (wxCommandEvent &event);
	void OnCalcSN (wxCommandEvent &event);
	void OnCalcRandSN (wxCommandEvent &event);
	void OnTestSN (wxCommandEvent &event);
	void OnTestRandSN (wxCommandEvent &event);
	void OnCalcRSA (wxCommandEvent &event);
	void OnChangeSize (wxSizeEvent &event);
	enum {
		IDC_MACH_FINGER_PRINT = 1000,
		IDC_TEST_SN,
		IDC_TEST_RAND_SN,
		IDC_SOFTWARE_SN,
		IDC_SOFTWARE_RAND_SN,
		IDC_CALC_SN,
		IDC_CALC_RAND_SN,
		IDC_CALC_RSA_KEYS,
	};

	wxPanel	*		m_pPanel;
	wxTextCtrl *	m_pFingerPrt;
	wxTextCtrl *	m_SerialNo;
	wxTextCtrl *	m_RandSerialNo;

	wxTextCtrl *	m_pDKeys;
	wxTextCtrl *	m_pEKeys;
	wxTextCtrl *	m_pNValue;

	DECLARE_EVENT_TABLE ()
}; // class GmRegisterFrame


class GmRegisterApp : public wxApp
{
public:
	GmRegisterApp ();
	~GmRegisterApp ();
protected: // overriden methods
	/**
	 * 初始化应用程序。
	 */
    virtual bool OnInit (void);
    
    /**
     * 退出应用程序，并清理应用程序所占用的资源。
     */
	virtual int OnExit (void);

protected:
	wxLocale * m_pLocale;
}; // End of class abRestoreApp


#endif //__GM_APP_FRAME_H__