#include <cstdio>
#include <cstring>
#include <string>
#include <deque>
#include <filesystem>
#include <unistd.h>

// 行の末尾には '\n' を含む
// エラー時には std::runtime_error を発生させる
std::deque<std::string> GetTailLines(FILE *fp, const size_t numberOfTailLines)
{
    if (!fp)
    {
        throw std::runtime_error("fp nullptr error");
    }

    if (numberOfTailLines == 0)
    {
        return std::deque<std::string>();
    }

    std::deque<std::string> lineBufs;
    // deque::back() は empty だと実行できないので、あらかじめ追加する
    lineBufs.push_back("");

    while (true)
    {
        const auto ch = fgetc(fp);
        if (ch == EOF)
        {
            break;
        }

        const bool moveToNextLine = !lineBufs.back().empty() && lineBufs.back().back() == '\n';
        if (moveToNextLine)
        {
            // 行バッファを使い切っていたら、一番古い行を削除する
            if (lineBufs.size() == numberOfTailLines)
            {
                lineBufs.pop_front();
            }
            lineBufs.push_back("");
        }

        lineBufs.back() += ch;
    }

    if (ferror(fp) != 0)
    {
        throw std::runtime_error("fgetc error");
    }

    return lineBufs;
}

void PrintLines(const std::deque<std::string> &lines)
{
    for (const auto &line : lines)
    {
        printf("%s", line.c_str());
    }
}

// エラー時には std::runtime_error を発生させる
void TailFile(const std::filesystem::path &path, const size_t numberOfTailLines)
{
    auto fp = fopen(path.c_str(), "r");
    if (!fp)
    {
        const std::string err = "fopen error, " + std::string(std::strerror(errno));
        throw std::runtime_error(err);
    }

    std::deque<std::string> lines;
    try
    {
        lines = std::move(GetTailLines(fp, numberOfTailLines));
    }
    catch (const std::runtime_error &e)
    {
        fclose(fp);
        const std::string err = "GetTailLines error, " + std::string(e.what());
        throw std::runtime_error(err);
    }

    fclose(fp);
    PrintLines(lines);
}

// エラー時には std::runtime_error を発生させる
void TailStdIn(const size_t numberOfTailLines)
{
    std::deque<std::string> lines;
    try
    {
        lines = std::move(GetTailLines(stdin, numberOfTailLines));
    }
    catch (const std::runtime_error &e)
    {
        const std::string err = "GetTailLines error, " + std::string(e.what());
        throw std::runtime_error(err);
    }

    PrintLines(lines);
}

struct tailOpt final
{
    size_t tailLines = 5;
    std::filesystem::path path;
};

// エラー時には std::runtime_error を発生させる
tailOpt ParseOpt(const int argc, char *argv[])
{
    tailOpt opt;
    while (true)
    {
        // ロングオプションを使用するには getopt_long を使う
        const auto op = getopt(argc, argv, "n:");
        if (op == -1)
        {
            break;
        }

        switch (op)
        {
        case 'n':
            try
            {
                opt.tailLines = std::stoul(optarg);
            }
            catch (const std::exception &e)
            {
                const std::string err = "stoul error, " + std::string(e.what());
                throw std::runtime_error(err);
            }
            break;
        // 'n' 以外は '?' になる
        case '?':
            break;
        }
    }

    // getopt は argv を書き換える
    // getopt 実行前: main.cpp -n 6 -a main.cpp -h aaa.cpp
    // getopt 実行後: main.cpp -n 6 -a -h main.cpp aaa.cpp
    // オプション以外の引数が存在するときはファイルパスとする
    // 一番始めに出現したものだけ処理する
    if (optind < argc)
    {
        opt.path = argv[optind];
    }

    return opt;
}

int main(int argc, char *argv[])
{
    tailOpt opt;
    try
    {
        opt = ParseOpt(argc, argv);
    }
    catch (const std::runtime_error &e)
    {
        fprintf(stderr, "ParseOpt error, %s\n", e.what());
        return EXIT_FAILURE;
    }

    if (opt.path.empty())
    {
        try
        {
            TailStdIn(opt.tailLines);
            return EXIT_SUCCESS;
        }
        catch (const std::runtime_error &e)
        {
            fprintf(stderr, "TailStdIn error, %s\n", e.what());
            return EXIT_FAILURE;
        }
    }

    try
    {
        TailFile(opt.path, opt.tailLines);
        return EXIT_SUCCESS;
    }
    catch (const std::runtime_error &e)
    {
        fprintf(stderr, "TailFile error, %s\n", e.what());
        return EXIT_FAILURE;
    }
}
