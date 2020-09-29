#include <cstdio>
#include <string>
#include <string.h>
#include <deque>
#include <filesystem>

// エラー時には std::runtime_error を発生させる
std::deque<std::string> GetTailLines(FILE *fp, const size_t numberOfTailLines)
{
    if (!fp)
    {
        throw std::runtime_error("fp nullptr error");
    }

    std::deque<std::string> lineBufs;
    while (true)
    {
        const auto ch = fgetc(fp);
        if (ch == EOF)
        {
            break;
        }

        if (ch == '\n')
        {
            // 行バッファを使い切っていたら、一番古い行を削除する
            if (lineBufs.size() == numberOfTailLines)
            {
                lineBufs.pop_front();
            }
            lineBufs.push_back("");
            continue;
        }

        // back() は empty だと実行できないので、空のときはあらかじめ追加する
        if (lineBufs.empty())
        {
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
        printf("%s\n", line.c_str());
    }
}

// エラー時には std::runtime_error を発生させる
void TailFile(const std::filesystem::path &path, const size_t numberOfTailLines)
{
    auto fp = fopen(path.c_str(), "r");
    if (!fp)
    {
        const std::string err = "fopen error, " + std::string(strerror(errno));
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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        try
        {
            TailStdIn(5);
            return 0;
        }
        catch (const std::runtime_error &e)
        {
            fprintf(stderr, "TailStdIn error, %s\n", e.what());
            return 1;
        }
    }
    else
    {
        try
        {
            TailFile(argv[1], 5);
            return 0;
        }
        catch (const std::runtime_error &e)
        {
            fprintf(stderr, "TailFile error, %s\n", e.what());
            return 1;
        }
    }
}
