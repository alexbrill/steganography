#include "stdafx.h"
#include <helpers/file_list_helper.h>
#include <helpers/filetimetools.h>


namespace { // anon namespace local classes for good measure
	class tpc_DecodeFiles : public threaded_process_callback {
	public:
		tpc_DecodeFiles ( metadb_handle_list_cref items, const char * pathTo ) : m_pathTo(pathTo), m_outFS(filesystem::get(pathTo)) {
			m_lstFiles.init_from_list( items );
		}
		void on_init(HWND p_wnd) override {	}

		void run(threaded_process_status & p_status, abort_callback & p_abort) override {
			for( size_t fileWalk = 0; fileWalk < m_lstFiles.get_size(); ++ fileWalk ) {

				p_abort.check(); 

				const char * inPath = m_lstFiles[fileWalk];
				p_status.set_progress(fileWalk, m_lstFiles.get_size());
				p_status.set_item_path(inPath);

				try {
					workWithFile(inPath, p_abort);
				} catch(exception_aborted) {
					throw;
				} catch(std::exception const & e) {
					m_errorLog << "Could not decode: " << file_path_display(inPath) << ", reason: " << e << "\n";
				}				
			}
		}

		void workWithFile( const char * inPath, abort_callback & abort ) {
			FB2K_console_formatter() << "File: " << file_path_display(inPath);

			const filesystem::ptr inFS = filesystem::get( inPath );
			pfc::string8 inFN;
			inFS->extract_filename_ext(inPath, inFN);
			pfc::string8 outPath = m_pathTo;
			outPath.end_with( m_outFS->pathSeparator() );
			pfc::string8 outPathDecode = outPath;

			console::print("THIS IS IT");
			console::print(file_path_display(inPath)); // À ÑÞÄÀ 
			console::print(file_path_display(outPathDecode)); //ÏÓÒÜ ÊÓÄÀ ÈÄÅÒ ÊÎÏÈÐÎÂÀÍÈÅ ÕÅ ÕÅ 

			lua_State* L = luaL_newstate();
			luaL_dofile(L, "Player.lua");
			luaL_openlibs(L);
			lua_pcall(L, 0, 0, 0);

			using namespace luabridge;
			
			std::string in = file_path_display(inPath);
			std::string out = file_path_display(outPathDecode);

			LuaRef Encode = getGlobal(L, "DecodeToFile");

			 console::print(Encode(in, out));
		}
		
		void on_done(HWND p_wnd, bool p_was_aborted) override {
			if (! p_was_aborted && m_errorLog.length() > 0 ) {
				popup_message::g_show(m_errorLog, "Information");
			}

		}

		file_list_helper::file_list_from_metadb_handle_list m_lstFiles;
		const pfc::string8 m_pathTo;
		const filesystem::ptr m_outFS;
		pfc::string_formatter m_errorLog;
	};
}

void RunDecodeFiles(metadb_handle_list_cref data) {
	if (!ModalDialogPrologue()) return;

	const HWND wndParent = core_api::get_main_window();
	pfc::string8 DecodeTo;

	if (!uBrowseForFolder( wndParent, "Choose destination folder", DecodeTo )) return;
	
	pfc::string8 DecodeTo2 = PFC_string_formatter() << "file://" << DecodeTo;

	auto worker = fb2k::service_new<tpc_DecodeFiles>(data, DecodeTo2);
	const uint32_t flags = threaded_process::flag_show_abort | threaded_process::flag_show_progress | threaded_process::flag_show_item;
	threaded_process::get()->run_modeless( worker, flags, wndParent, "DECODING PROCESS" );
}

