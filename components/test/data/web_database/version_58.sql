PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE meta(key LONGVARCHAR NOT NULL UNIQUE PRIMARY KEY, value LONGVARCHAR);
INSERT INTO "meta" VALUES('version','57');
INSERT INTO "meta" VALUES('last_compatible_version','57');
INSERT INTO "meta" VALUES('Builtin Keyword Version','70');
INSERT INTO "meta" VALUES('Default Search Provider ID','2');
CREATE TABLE web_intents ( service_url LONGVARCHAR, action VARCHAR, type VARCHAR, title LONGVARCHAR, disposition VARCHAR, scheme VARCHAR, UNIQUE (service_url, action, scheme, type));
CREATE TABLE web_intents_defaults ( action VARCHAR, type VARCHAR, url_pattern LONGVARCHAR, user_date INTEGER, suppression INTEGER, service_url LONGVARCHAR, scheme VARCHAR, UNIQUE (action, scheme, type, url_pattern));
CREATE TABLE keywords (id INTEGER PRIMARY KEY,short_name VARCHAR NOT NULL,keyword VARCHAR NOT NULL,favicon_url VARCHAR NOT NULL,url VARCHAR NOT NULL,safe_for_autoreplace INTEGER,originating_url VARCHAR,date_created INTEGER DEFAULT 0,usage_count INTEGER DEFAULT 0,input_encodings VARCHAR,show_in_default_list INTEGER,suggest_url VARCHAR,prepopulate_id INTEGER DEFAULT 0,created_by_policy INTEGER DEFAULT 0,instant_url VARCHAR,last_modified INTEGER DEFAULT 0,sync_guid VARCHAR,alternate_urls VARCHAR,search_terms_replacement_key VARCHAR,image_url VARCHAR,search_url_post_params VARCHAR,suggest_url_post_params VARCHAR,instant_url_post_params VARCHAR,image_url_post_params VARCHAR,new_tab_url VARCHAR);
INSERT INTO "keywords" VALUES(2,'Google','google.com','http://www.google.com/favicon.ico','{google:baseURL}search?q={searchTerms}&{google:RLZ}{google:originalQueryForSuggestion}{google:assistedQueryStats}{google:searchFieldtrialParameter}{google:bookmarkBarPinned}{google:searchClient}{google:sourceId}{google:instantExtendedEnabledParameter}{google:omniboxStartMarginParameter}ie={inputEncoding}',1,'',0,0,'UTF-8',1,'{google:baseSuggestURL}search?{google:searchFieldtrialParameter}client={google:suggestClient}&xssi=t&q={searchTerms}&{google:cursorPosition}{google:zeroPrefixUrl}{google:pageClassification}sugkey={google:suggestAPIKeyParameter}',1,0,'{google:baseURL}webhp?sourceid=chrome-instant&{google:RLZ}{google:forceInstantResults}{google:instantExtendedEnabledParameter}{google:ntpIsThemedParameter}{google:omniboxStartMarginParameter}ie={inputEncoding}',0,'1D955A9B-6F30-D3FF-5F5C-002E40BDF482','["{google:baseURL}#q={searchTerms}","{google:baseURL}search#q={searchTerms}","{google:baseURL}webhp#q={searchTerms}"]','espv','{google:baseURL}searchbyimage/upload','','','','encoded_image={google:imageThumbnail},image_url={google:imageURL},sbisrc={google:imageSearchSource},original_width={google:imageOriginalWidth},original_height={google:imageOriginalHeight}','{google:baseURL}_/chrome/newtab?{google:RLZ}{google:instantExtendedEnabledParameter}{google:ntpIsThemedParameter}ie={inputEncoding}');
INSERT INTO "keywords" VALUES(3,'Bing','bing.com','http://www.bing.com/s/wlflag.ico','http://www.bing.com/search?setmkt=en-US&q={searchTerms}',1,'',0,0,'UTF-8',1,'http://api.bing.com/osjson.aspx?query={searchTerms}&language={language}',3,0,'',0,'E88DEF4C-20E0-5DD7-6A71-20058C05BF09','[]','','','','','','','');
INSERT INTO "keywords" VALUES(4,'Yahoo!','yahoo.com','http://search.yahoo.com/favicon.ico','http://search.yahoo.com/search?ei={inputEncoding}&fr=crmas&p={searchTerms}',1,'',0,0,'UTF-8',1,'http://ff.search.yahoo.com/gossip?output=fxjson&command={searchTerms}',2,0,'',0,'F2F457FC-8B48-47A4-7D7C-8001AB91D1BD','[]','','','','','','','');
INSERT INTO "keywords" VALUES(5,'AOL','aol.com','http://search.aol.com/favicon.ico','http://search.aol.com/aol/search?query={searchTerms}',1,'',0,0,'UTF-8',1,'http://autocomplete.search.aol.com/autocomplete/get?output=json&it=&q={searchTerms}',35,0,'',0,'F858668C-43E0-5CF3-7D05-EDE8DBF40106','[]','','','','','','','');
INSERT INTO "keywords" VALUES(6,'Ask','ask.com','http://sp.ask.com/sh/i/a16/favicon/favicon.ico','http://www.ask.com/web?q={searchTerms}',1,'',0,0,'UTF-8',1,'http://ss.ask.com/query?q={searchTerms}&li=ff',4,0,'',0,'00268D17-EC55-D173-84F3-48D9B862E8E1','[]','','','','','','','');
INSERT INTO "keywords" VALUES(264,'Tab Extract','ex','','chrome-extension://iphchnegaodmijmkdlbhbanjhfphhikp/?q={searchTerms}',0,'',1415010415,0,'',0,'',0,0,'',1415010415,'AAA4A0C0-5B9A-4347-B0DA-837C12F5CFE9','[]','','','','','','','');
CREATE TABLE web_app_icons (url LONGVARCHAR,width int,height int,image BLOB, UNIQUE (url, width, height));
CREATE TABLE web_apps (url LONGVARCHAR UNIQUE,has_all_images INTEGER NOT NULL);
CREATE TABLE autofill (name VARCHAR, value VARCHAR, value_lower VARCHAR, date_created INTEGER DEFAULT 0, date_last_used INTEGER DEFAULT 0, count INTEGER DEFAULT 1, PRIMARY KEY (name, value));
CREATE TABLE credit_cards ( guid VARCHAR PRIMARY KEY, name_on_card VARCHAR, expiration_month INTEGER, expiration_year INTEGER, card_number_encrypted BLOB, date_modified INTEGER NOT NULL DEFAULT 0, origin VARCHAR DEFAULT '');
CREATE TABLE autofill_profiles ( guid VARCHAR PRIMARY KEY, company_name VARCHAR, street_address VARCHAR, dependent_locality VARCHAR, city VARCHAR, state VARCHAR, zipcode VARCHAR, sorting_code VARCHAR, country_code VARCHAR, date_modified INTEGER NOT NULL DEFAULT 0, origin VARCHAR DEFAULT '', language_code VARCHAR);
INSERT INTO "autofill_profiles" VALUES('00000000-0000-0000-0000-000000000001','Google Inc','340 Main St','','Los Angeles','CA','90291','','US',1395948829,'Chrome settings', 'en-US');
CREATE TABLE autofill_profile_names ( guid VARCHAR, first_name VARCHAR, middle_name VARCHAR, last_name VARCHAR, full_name VARCHAR);
INSERT INTO "autofill_profile_names" VALUES('B41FE6E0-B13E-2A2A-BF0B-29FCE2C3ADBD','Jon','','Smith', 'Jon Smith');
CREATE TABLE autofill_profile_emails ( guid VARCHAR, email VARCHAR);
INSERT INTO "autofill_profile_emails" VALUES('B41FE6E0-B13E-2A2A-BF0B-29FCE2C3ADBD','');
CREATE TABLE autofill_profile_phones ( guid VARCHAR, number VARCHAR);
INSERT INTO "autofill_profile_phones" VALUES('B41FE6E0-B13E-2A2A-BF0B-29FCE2C3ADBD','');
CREATE TABLE autofill_profiles_trash ( guid VARCHAR);
CREATE TABLE token_service (service VARCHAR PRIMARY KEY NOT NULL,encrypted_token BLOB);
CREATE INDEX web_intents_index ON web_intents (action);
CREATE INDEX web_intents_default_index ON web_intents_defaults (action);
CREATE INDEX web_apps_url_index ON web_apps (url);
CREATE INDEX autofill_name ON autofill (name);
CREATE INDEX autofill_name_value_lower ON autofill (name, value_lower);
COMMIT;
