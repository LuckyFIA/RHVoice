/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <exception>
#include "init.hpp"
#include "io.hpp"

namespace RHVoice
{
  namespace sd
  {
    namespace cmd
    {
      bool init::is_valid() const
      {
        return tts_engine.empty();
      }

      action_t init::execute()
      {
        try
          {
            tts_engine=engine::create();
            logger::log(3,"Initialized the engine");
            register_languages();
            register_voices();
            synthesizer.start();
            logger::log(2,"Started the synthesis thread");
          }
        catch(const std::exception& e)
          {
            logger::log(2,"Initialization error: ",e.what());
            reply r;
            r("399-",e.what());
            r("399 ERROR CANNOT INITIALIZE MODULE");
            return action_abort;
          }
        reply r;
        r("299-RHVoice initialized");
        r("299 OK LOADED SUCCESSFULLY");
        return action_continue;
      }

      void init::register_languages()
      {
        std::string code;
        const language_list& languages=tts_engine->get_languages();
        for(language_list::const_iterator it=languages.begin();it!=languages.end();++it)
          {
            logger::log(4,it->get_name()," language supported");
            code=it->get_alpha2_code();
            if(code.empty())
              code=it->get_alpha3_code();
            if(!code.empty())
              tts_settings.language_code.define(code);
          }
      }

      void init::register_voices()
      {
        voice_description voice_desc;
        const voice_list& available_voices=tts_engine->get_voices();
        for(voice_list::const_iterator it1=available_voices.begin();it1!=available_voices.end();++it1)
          {
            logger::log(4,"Found ",it1->get_language()->get_name()," speaker ",it1->get_name());
            voice_desc=voice_description(it1);
            voices[voice_desc.get_name()]=voice_desc;
            for(voice_list::const_iterator it2=available_voices.begin();it2!=available_voices.end();++it2)
              {
                if(!(it1->get_language()->has_common_letters(*(it2->get_language()))))
                  {
                    voice_desc=voice_description(it1,it2);
                    voices[voice_desc.get_name()]=voice_desc;
                  }
              }
          }
        for(voice_map::const_iterator it=voices.begin();it!=voices.end();++it)
          {
            tts_settings.voice_name.define(it->first);
          }
      }
    }
  }
}