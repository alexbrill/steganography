#include "stdafx.h"
#include "resource.h"
#include "stdafx.h"
#include <helpers/file_list_helper.h>
#include <helpers/filetimetools.h>
#include <mutex>



std::mutex file_mutex;

namespace { 
	class tpc_DecodeFiles : public threaded_process_callback {
	public:
		tpc_DecodeFiles(metadb_handle_list_cref items)  {
			m_lstFiles.init_from_list(items);
		}

		void on_init(HWND p_wnd) override {	}

		void run(threaded_process_status & p_status, abort_callback & p_abort) override {
			for (size_t fileWalk = 0; fileWalk < m_lstFiles.get_size(); ++fileWalk) {

				p_abort.check();

				const char * inPath = m_lstFiles[fileWalk];
				p_status.set_progress(fileWalk, m_lstFiles.get_size());
				p_status.set_item_path(inPath);

				try {
					workWithFile(inPath, p_abort);
				}
				catch (exception_aborted) {
					throw;
				}
				catch (std::exception const & e) {
					m_errorLog << "Could not decode: " << file_path_display(inPath) << ", reason: " << e << "\n";
				}
			}
		}

		void workWithFile(const char * inPath, abort_callback & abort) {

			const filesystem::ptr inFS = filesystem::get(inPath);
			pfc::string8 inFN;
			inFS->extract_filename_ext(inPath, inFN);

			if (inFN.has_suffix("wav"))
			{
				console::print("DECODING");
				lua_State* L = luaL_newstate();
				luaL_dofile(L, "Player.lua");
				luaL_openlibs(L);
				lua_pcall(L, 0, 0, 0);

				std::string in = file_path_display(inPath);

				luabridge::LuaRef Encode = luabridge::getGlobal(L, "DecodeToFile");
				console::print(Encode(in));
			}
		}

		void on_done(HWND p_wnd, bool p_was_aborted) override {

			file_mutex.unlock();

			if (!p_was_aborted && m_errorLog.length() > 0) {
				popup_message::g_show(m_errorLog, "Information");
			}
		}

		file_list_helper::file_list_from_metadb_handle_list m_lstFiles;
		const pfc::string8 m_pathTo;
		pfc::string_formatter m_errorLog;
	};
}

void RunDecodeFiles(metadb_handle_ptr data);

class Foo : public play_callback_static {
public:
	virtual unsigned get_flags() { return  flag_on_playback_new_track; }
	virtual void on_playback_starting(play_control::t_track_command p_command, bool p_paused) {  }	
	virtual void on_playback_stop(play_control::t_stop_reason p_reason) {  }
	virtual void on_playback_seek(double p_time) {}
	virtual void on_playback_pause(bool p_state) {}
	virtual void on_playback_edited(metadb_handle_ptr p_track) {}
	virtual void on_playback_dynamic_info(const file_info & p_info) {}
	virtual void on_playback_dynamic_info_track(const file_info & p_info) {}
	virtual void on_playback_time(double p_time) {}
	virtual void on_volume_change(float p_new_val) {}

	virtual void on_playback_new_track(metadb_handle_ptr p_track) {
		console::print("START");
		RunDecodeFiles(p_track);
	}
};

static play_callback_static_factory_t<Foo> bar;


void RunDecodeFiles(metadb_handle_ptr data) {
	if (file_mutex.try_lock())
	{
		pfc::list_single_ref_t<metadb_handle_ptr> a(data);
		const HWND wndParent = core_api::get_main_window();
		auto worker = fb2k::service_new<tpc_DecodeFiles>(a);
		uint32_t flags = threaded_process::flag_show_abort | threaded_process::flag_show_progress | threaded_process::flag_show_item;
		threaded_process::get()->run_modeless(worker, flags, wndParent, "DECODING PROCESS");
	}
}
