#include "picojson.h"
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

using namespace std;

picojson::value schedule,sessions,papers,extra;

static void readJSON(picojson::value& tree,string file,string tag)
{
  ifstream in(file);
  if (!in.is_open()) {
    cerr << "unable to open " << file << endl;
    exit(1);
  }
  string data,line;
  while (true) {
    if (!getline(in,line)) {
      cerr << "malformed file " << file << endl;
      exit(1);
    }
    if ((line.length()>tag.length())&&(line.substr(0,tag.length())==tag)&&(line[tag.length()]=='=')) {
      data+=line.substr(tag.length()+1);
      break;
    }
  }
  while (getline(in,line)) {
    data+="\n";
    data+=line;
  }

  string err;
  picojson::parse(tree, data.begin(), data.end(), &err);
  if (!err.empty()) {
    cerr << err << endl;
    exit(1);
  }
}

static void readData()
{
  readJSON(schedule,"confer/schedule.json","schedule");
  readJSON(sessions,"confer/sessions.json","sessions");
  readJSON(papers,"confer/papers.json","entities");
  readJSON(extra,"confer/extra.json","extra");
}

static string escapeLatex(string s)
{
  string result;
  for (auto c:s) {
    switch (c) {
      case '#': result+="\\#"; break;
      case '$': result+="\\$"; break;
      case '%': result+="\\%"; break;
      case '&': result+="\\&"; break;
      case '\\': result+="\\textbackslash{}"; break;
      case '^': result+="\\textasciicircum{}"; break;
      case '_': result+="\\_"; break;
      case '{': result+="\\{"; break;
      case '}': result+="\\}"; break;
      case '~': result+="\\textasciitilde{}"; break;
      default: result+=c; break;
    }
  }
  return result;
}

static string handleHTML(string s)
{
  while (true) {
     auto p=s.find("&nbsp;");
     if (p==string::npos) break;
     s=s.substr(0,p)+" "+s.substr(p+6);
  }
  while (true) {
     auto p=s.find("&amp;");
     if (p==string::npos) break;
     s=s.substr(0,p)+"&"+s.substr(p+5);
  }
  return s;
}

static string fixUtf8(string s)
{
  while (true) {
     auto p=s.find("\x75\xCC\x88");
     if (p==string::npos) break;
     s=s.substr(0,p)+u8"ü"+s.substr(p+3);
  }
  return s;
}

static string pickSessionLabel(set<string>& sessionsSeen,string name)
{
   if (!sessionsSeen.count(name)) {
      sessionsSeen.insert(name);
      return escapeLatex(name);
   }
   for (unsigned index=2;;++index) {
      auto tmp=name+":"+to_string(index);
      if (!sessionsSeen.count(tmp)) {
         sessionsSeen.insert(tmp);
         return escapeLatex(tmp);
      }
   }
}

static void dumpSession(ostream& out,string room,string sessionName,string sessionLabel)
{
  auto& s=extra.get<picojson::object>()["specialsessions"].get<picojson::object>();
  if (s.count(sessionName)&&(s[sessionName].get<picojson::object>().count("detailed"))) {
     out << s[sessionName].get<picojson::object>()["detailed"].get<string>() << endl;
     return;
  } else if (!sessions.contains(sessionName)) {
     cerr << "warning: skipping session " << sessionName << ", no entry found" << endl;
     return;
  }
  auto session=sessions.get<picojson::object>()[sessionName].get<picojson::object>();
  out << "\\sessionname{" << sessionLabel << "}{" << escapeLatex(handleHTML(session["s_title"].get<string>())) << "}\\\\\n";
  out << "\\sessionlocation{" << escapeLatex(room) << "}\n\n";
  if (session.count("chair"))
     out << "\\sessionchair{" << escapeLatex(session["chair"].get<string>()) << "}\n\n";
  out << "\\sessionsep{}\n";

  {
    auto& s=extra.get<picojson::object>()["sessionintro"].get<picojson::object>();
    if (s.count(sessionName)) {
       out << s[sessionName].get<string>() << endl << endl;
    }
  }

  for (auto submission_:session["submissions"].get<picojson::array>()) {
     auto paperName=submission_.get<string>();
     if (!papers.contains(paperName)) {
        cerr << "warning: skipping paper " << paperName << ", not found" << endl;
        continue;
     }
     auto paper=papers.get<picojson::object>()[paperName].get<picojson::object>();
     auto type=paper["type"].get<string>();
     if (type=="break") continue;
     out << "\\papertitle{" << escapeLatex(paper["title"].get<string>()) << "}{" << (paper.count("acm_link")?paper["acm_link"].get<string>():"") << "}\n";
     if (type=="industrial")
        out << " (industrial)";
     if (paper.count("authors")) {
        out << "\\paperauthors{";
        bool first=true;
        for (auto author_:paper["authors"].get<picojson::array>()) {
          auto author=author_.get<picojson::object>();
          if (first) first=false; else out << ", ";
          out << escapeLatex(fixUtf8(author["name"].get<string>()));
          if (author.count("affiliation"))
             out << " (" << escapeLatex(fixUtf8(author["affiliation"].get<string>())) << ")";
        }
        out << "}\n";
     }
     out << "\n";

     if ((type=="awards")||(type=="keynote")||(type=="plenary")||(type=="workshop")||(type=="interactive session")) {
        out << escapeLatex(paper["abstract"].get<string>()) << "\n\n";
        if ((type=="keynote")||(type=="plenary")) {
           auto bios=extra.get<picojson::object>()["bios"].get<picojson::object>();
           if (bios.count(paperName))
              out << escapeLatex(bios[paperName].get<string>()) << "\n\n";
        }
     }
  }
}

static set<string> getIgnoreDays()
{
  set<string> ignoreDays;
  {
     auto& i=extra.get<picojson::object>()["ignoreday"].get<picojson::array>();
     for (auto& d:i)
        ignoreDays.insert(d.get<string>());
  }
  return ignoreDays;
}

static void dumpDetailed(string outFile)
{
  stringstream out;
  set<string> ignoreDays=getIgnoreDays(),sessionsSeen;
  auto& sa=schedule.get<picojson::array>();
  for (auto& day_:sa) {
    auto day=day_.get<picojson::object>();
    if (ignoreDays.count(day["date"].get<string>()))
       continue;
    string dayDate=day["day"].get<string>()+" "+day["date"].get<string>();
    for (auto& slot_:day["slots"].get<picojson::array>()) {
      auto slot=slot_.get<picojson::object>();
      string time=slot["time"].get<string>();

      out << "\\slotheading{" << dayDate << " " << time << "}" << endl << endl;

      for (auto& session_:slot["sessions"].get<picojson::array>()) {
	auto session=session_.get<picojson::object>();
        auto sessionName=session["session"].get<string>();
	dumpSession(out,session["room"].get<string>(),sessionName,pickSessionLabel(sessionsSeen,sessionName));
      }

    }
  }

  ofstream fout(outFile);
  if (!fout.is_open()) {
    cerr << "unable to write " << outFile << endl;
    exit(1);
  }
  fout << out.str();
}

static void dumpOverview(string outFile)
{
  stringstream out;
  set<string> ignoreDays=getIgnoreDays(),sessionsSeen;
  auto& sa=schedule.get<picojson::array>();
  map<string,map<string,map<string,string>>> schedule;
  for (auto& day_:sa) {
    auto day=day_.get<picojson::object>();
    if (ignoreDays.count(day["date"].get<string>()))
       continue;
    auto& d=schedule[day["date"].get<string>()];

    for (auto& slot_:day["slots"].get<picojson::array>()) {
      auto slot=slot_.get<picojson::object>();
      string time=slot["time"].get<string>();

      for (auto& session_:slot["sessions"].get<picojson::array>()) {
        auto session=session_.get<picojson::object>();
        auto sessionName=session["session"].get<string>();
        auto& s=extra.get<picojson::object>()["specialsessions"].get<picojson::object>();
        if (s.count(sessionName)) {
          d[time][""]=s[sessionName].get<picojson::object>()["overview"].get<string>();
          continue;
        } else if (!sessions.contains(sessionName)) {
          cerr << "warning: skipping session " << sessionName << ", no entry found" << endl;
          continue;
        }
        auto label=pickSessionLabel(sessionsSeen,sessionName);
        d[time][session["room"].get<string>()]="\\hyperlink{"+label+"}{"+escapeLatex(handleHTML(sessions.get<picojson::object>()[sessionName].get<picojson::object>()["s_title"].get<string>()))+"}";
      }
    }
  }

  vector<string> rooms;
  {
     auto& rl=extra.get<picojson::object>()["roomlist"].get<picojson::array>();
     for (auto& r:rl)
        rooms.push_back(r.get<string>());
  }

  auto& da=extra.get<picojson::object>()["datelist"].get<picojson::array>();
  for (auto& day:da) {
     out << R"({\resizebox{\linewidth}{!}{)" << endl << R"(\begin{overviewtable}{)" << day.get<picojson::object>()["overview"].get<string>() << "}" << endl;
     for (auto& time_:schedule[day.get<picojson::object>()["date"].get<string>()]) {
        auto time=time_.first;
        auto& t=time_.second;
        out << time;
        if (t.count("")) {
           out << R"( & \multicolumn{7}{|c|}{)" << t[""] << "}";
        } else {
           for (auto& r:rooms) {
              out << " & ";
              if (t.count(r)) {
                 if (time.substr(0,6)=="08:30-") {
                    out << R"(\multicolumn{7}{|c|}{\overviewplenary{)" << t[r] << "}}";
                    break;
                 }
                 out << t[r];
              }
           }
        }
        out << R"( \\\hline)" << endl;
     }
     out << R"(\end{overviewtable})" << endl << R"(}})" << endl << endl;
  }

  ofstream fout(outFile);
  if (!fout.is_open()) {
    cerr << "unable to write " << outFile << endl;
    exit(1);
  }
  fout << out.str();
}


int main(int argc,char** argv)
{
  if (argc!=3) {
    cerr << "usage: " << argv[0] << " command output" << endl;
    return 1;
  }

  readData();

  string command=argv[1];
  if (command=="detailed") {
    dumpDetailed(argv[2]);
  } else if (command=="overview") {
    dumpOverview(argv[2]);
  } else {
    cerr << "unknown command " << command << endl;
    return 1;
  }
}
