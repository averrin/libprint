#include "libprint.hpp"
#include <string>
#include <thread>

using namespace LibPrint;

void formatting() {
  utils::h1("FORMATTING");
  fmt::print("        {}\n", utils::bold("bold string"));
  fmt::print("        {}\n", utils::italic("italic string"));
  fmt::print("        {}\n", utils::underline("underline string"));
  fmt::print("        {}\n", utils::strikethrough("strikethrough string"));

  fmt::print("\n");
  fmt::print("colored rgb: {}, terminal: {}\n",
             utils::color(fmt::rgb(0, 255, 0), "green"), utils::green("green"));
  fmt::print("nested       {}\n",
             utils::underline("{}+{}={}", utils::red("{}", 2),
                              utils::blue("{}", 2), utils::green("{}", 2 + 2)));

  fmt::print(
      "nested (fmt) {}",
      fmt::format(
          fmt::emphasis::underline, "{}+{}={}",
          fmt::format(fmt::fg(fmt::terminal_color::red), "{}", 2),
          fmt::format(fmt::fg(fmt::terminal_color::blue), "{}", 2),
          fmt::format(fmt::fg(fmt::terminal_color::green), "{}", 2 + 2)));
  fmt::print(
      utils::gray("  //same styling bug in original fmtlib implementation\n"));
}

void rules() {
  utils::h1("RULES");
  fmt::print(utils::rule(57));
  auto l = 50;
  for (int n = 0; n < 6; n++) {
    fmt::print("{:>5}% ", n * 20);
    fmt::print(utils::stacked(n * 10, l - n * 10, fmt::color::orange));
  }
  fmt::print(
      utils::stacked(25, 25, fmt::color::white, fmt::color::dark_gray, false));
  fmt::print(utils::rule(7, fmt::color::dim_gray));

  utils::h2("Headers");
  utils::h1("H1");
  fmt::print("With blank lines before and after\n");
  utils::h2("H2");
  fmt::print("With blank lines before and after\n");
  utils::h3("H3");
  fmt::print("Without blank line\n");
}

void markup() {
  utils::h1("MARKUP");
  utils::h2("Style");
  fmt::print(utils::parse(
      "<b>bold</b> <u>underline</u> <i>italic</i> <s>strike</s>\n"));
  utils::h2("Terminal colors");
  fmt::print(utils::parse(
      "<red>red</red> <black>black</black> "
      "<green>green</green> <blue>blue</blue> <yellow>yellow</yellow> "
      "<cyan>cyan</cyan> <gray>gray</gray>\n"));
  fmt::print("<b><red>red</red> <black>black</black> "
             "<green>green</green> <blue>blue</blue> <yellow>yellow</yellow> "
             "<cyan>cyan</cyan> <gray>gray</gray></b>\n"_p);

  utils::h2("Hex colors");
  fmt::print(
      "<b><color=#11ff11>#11ff11</color> "
      "<color=#ff1111>#ff1111</color></b> <color=#aa11bb>#aa11bb</color>\n"_p);
  fmt::print("<b><bgcolor=#11ff11>#11ff11</bgcolor> "
             "<bgcolor=#ff1111>#ff1111</bgcolor></b> "
             "<bgcolor=#aa11bb>#aa11bb</bgcolor>\n"_p);

  fmt::print("<b><bgcolor=#eeeeee>"
             "<color=#222222> black on white </color></bgcolor></b>"
             " <b><bgcolor=#11ff11>"
             "<color=#ff1111> red on green </color></bgcolor></b>\n"_p);

  utils::h2("Other");
  fmt::print("functions (no nesting):   ");
  fmt::print(utils::bold("{} {}\n", utils::green("green_bold"),
                         utils::red("red_bold")));
  fmt::print("fmt (no nesting):         ");
  fmt::print(fmt::format(
      fmt::emphasis::bold,
      fmt::format(
          "{} {}\n",
          fmt::format(fmt::fg(fmt::terminal_color::green), "green_bold"),
          fmt::format(fmt::fg(fmt::terminal_color::red), "red_bold"))));
  fmt::print("markup (nesting support): <b><green>green_bold</green> "
             "<red>red_bold</red></b>\n"_p);
  fmt::print("\n");
};

void comments() {

  auto cmt = CommentPrinter();
  utils::h2("Comments Printer");
  helpers::comment("comment line1", "comment line2");

  cmt.printBlock("this is multiline comment\nline1\n"
                 "<red>FIXME:</red> markup breaks gray color of the line\n"
                 "line3");

  cmt.fgColor = fmt::color::dark_green;
  cmt.mark = "#";
  cmt.println("python-like comment");
  utils::br();
}

void printer() {
  utils::h1("Printer examples");
  auto p = Printer();
  p.print("Simple printer print");
  utils::br();
  p.println("Simple printer println");
  p = Printer(3);
  p.print("Indented print (no gutter)");
  utils::br();
  p.println("Indented println");
  helpers::indent(6, "indent helper", "indented line 2", "indented line 3");

  utils::br();
  helpers::quote("quoted line", "quoted line 2");
  utils::br();
  helpers::aligned(Align::LEFT, 80, "<blue>left</blue> line", "left line 2");
  helpers::aligned(Align::MIDDLE, 80, "<green>middle</green> line",
                   "middle line 2");
  helpers::aligned(Align::RIGHT, 80, "<yellow>right</yellow> line",
                   "right line 2");
}

void gutter() {
  auto p = Printer(1);
  utils::h2("Gutter align");
  p.indent = 0;
  p.gutter.push(3, utils::green("┃"), Align::LEFT);
  p.println("left aligned gutter");
  p.gutter.push(Align::MIDDLE);
  p.println("middle aligned gutter");
  p.gutter.push(Align::RIGHT);
  p.println("right aligned gutter");
  p.indent = 1;
  helpers::comment("you need have gutter wider than 1");
  p.gutter.clear();

  utils::h2("Gutter examples");
  p.gutter.push(1, utils::green("┃"), Align::MIDDLE);
  p.println("line 1");
  p.markLine(utils::red("┣"), "line 2");
  p.println("line 3");
  p.gutter.clear();

  utils::br();
  utils::h3("Marks default");
  p.gutter.push(1);
  p.indent = 1;
  p.markLine("<b><blue>*</blue></b>"_p, "symbol marked line");
  p.println("not marked");
  p.markLine("<b><green>✓</blue></b>"_p, "checked with unicode symbol");
  p.markLine(fmt::color::cyan, "color marked line (empty gutter)");
  p.gutter.push("#");
  {
    p.println("set gutter content");
    p.markLine(fmt::color::blue, "color marked line (gutter with content)");
  }
  p.gutter.pop();

  utils::br();
  utils::h3("Marks left & right gutters");
  utils::br();
  auto gp = Printer(1);
  gp.rightGutter.enabled = true;
  gp.leftGutter.enabled = true;
  gp.println("not marked");
  gp.markLine(fmt::color::cyan, "color marked line");
  gp.markLine("<b><blue>*</blue></b>"_p, "symbol marked line");

  utils::br();
  utils::h3("More examples");
  utils::br();
  p.setGutter(Gutter(" ")); // reserve place for vertical line
  p.println(utils::rule(60, fmt::rgb(70, 70, 70), false, true));

  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
  auto tstamp = oss.str();
  p.println("<b><color=#ffd700>Header text</color></b>");
  p.println("<i><color=#505050>{}</color></i>", tstamp);
  p.println(utils::rule(60, fmt::rgb(70, 70, 70), false, true));

  p.gutter.push("│");
  {
    p.println("<b><color=#ffd700>Header text</color></b>");
    p.println("<i><color=#505050>{}</color></i>", tstamp);
  }
  p.gutter.pop();

  p.println(utils::rule(60, fmt::rgb(70, 70, 70), false, true));
  helpers::with_gutter({2, utils::magenta("│"), Align::LEFT},
                       "<b><color=#ffd700>Header text</color></b>",
                       fmt::format("<i><color=#505050>{}</color></i>", tstamp));
}

void numbered() {
  utils::h2("Numbered Printer");
  auto ln = NumberedPrinter();
  ln.rightGutter.enabled = true;
  ln.leftGutter.enabled = true;
  auto l = 12;
  for (int n = 0; n < l; n++) {
    if (n == 3) {
      ln.gutter.push(fmt::color::coral);
    } else if (n == 5) {
      ln.markLine("<b><red>❯</red></b>"_p,
                  "<bgcolor=#555555>marked line</bgcolor>");
      continue;
    } else if (n == 7) {
      ln.linenum = 100;
      ln.gutter.pop();
    } else if (n == 10) {
      ln.markLine(fmt::color::cyan, "color marked line");
      continue;
    } else if (n == 12) {
      ln.markLine(fmt::color::cyan, "*", "color & symbol marked line");
      continue;
    }
    ln.println("numbered line");
  }
  helpers::numbered(105, "and with helper");
}

void vt() {
  auto p = Printer();
  utils::h2("VT examples");
  p.println("<b><green>green line</green></b>");
  utils::up();
  p.println("XXXXX");

  p.println("<b><green>green line</green></b>");
  utils::up();
  utils::clearLine();
  p.println("<red>red line</red>");

  utils::saveCursor();
  p.print("<b><green>green line</green></b>");
  utils::restoreCursor();
  p.print("<b><blue>green line</blue></b>");
  utils::br();
}

void statusbar() {
  utils::h2("Printer with status bar");
  auto p = PrinterWithStatusBar();
  using namespace std::chrono;
  auto delay = 200ms;
  p.println("one");
  std::this_thread::sleep_for(delay);
  p.println("two");
  std::this_thread::sleep_for(delay);
  p.println("three");
  std::this_thread::sleep_for(delay);
  p.println("four");
  std::this_thread::sleep_for(delay);
  p.println("five");

  std::this_thread::sleep_for(delay);
  p.statusBarChars = utils::red("");
  p.rebuild();
  p.update();
  std::this_thread::sleep_for(delay);
  p.statusBarChars = utils::green("");
  p.rebuild();
  p.update();
  std::this_thread::sleep_for(delay);
  p.statusBarChars = utils::blue("");
  p.rebuild();
  p.update();
  std::this_thread::sleep_for(delay);
  p.statusBarChars = utils::cyan("");
  p.rebuild();
  p.update();

  auto total = 10;
  auto i = 0;
  for (i = 0; i < total; i++) {
    p.statusBar = utils::parse(fmt::format(
        "Printed lines: <yellow>{}</yellow>/<b><green>{}</green></b>", i + 1,
        total));
    p.println("one more line: {}", i);
    std::this_thread::sleep_for(delay);
  }
  for (i = total - 1; i >= 0; i--) {
    p.statusBar = utils::parse(fmt::format(
        "Printed lines: <yellow>{}</yellow>/<b><green>{}</green></b>", i,
        total));
    p.update();
    utils::saveCursor();
    utils::up(i + 2);
    utils::clearLine();
    utils::restoreCursor();
    std::this_thread::sleep_for(delay);
  }
}

int main() {
  // formatting();
  // rules();
  // markup();
  // printer();
  gutter();
  // numbered();
  // comments();
  // vt();
  // statusbar();

  auto p = Printer(1);
  p.gutter.enabled = true;
  p.gutter.push(utils::color(fmt::color::aqua, "  │      "));
  // p.println("{\n\t\"hum\": 40.22,\n\t\"lux\": 97.5,\n\t\"temp\": 22.91\n}\n");
  p.println("demo");
  p.gutter.pop();
}
