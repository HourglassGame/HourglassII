import imp
while True:
    import ci_agent_windows
    ci_agent_windows.main()
    imp.reload(ci_agent_windows)
