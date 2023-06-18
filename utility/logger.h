#pragma once

#include <string>


class Logger
{
public:
    enum Level
    {
        Verbose, 
        Info, 
        Warning, 
        Error,
        LevelSize
    };

    enum MsgType
    {
        Renderer,
        Loader,
        Platform,
        Editor,
        Others,
        MsgTypeSize
    };

    // TODO: behaviors for each Message type and level.
    enum Behavior
    {
        WriteToTerm   = 1 << 0,
        WriteToFS     = 1 << 1,
        WriteToTracy  = 1 << 2,
        ThrowNoError  = 1 << 3
    };

    static void SetLevel(Level minSeverity);

    /**
     * @brief Output the message with severity info.
     * If severity is error. The program exits.
     * 
     * @param msg 
     * @param level 
     * @param type 
     */
    static void Write(const std::string& msg,
        Level level = Level::Verbose, MsgType type = MsgType::Others);

private:
    static Level currentLevel;
};
