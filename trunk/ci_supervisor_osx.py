import imp
while True:
    import ci_agent_osx
    ci_agent_osx.main()
    imp.reload(ci_agent_osx)
