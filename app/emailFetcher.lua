require("email")
--very helpful project https://github.com/slembcke/debugger.lua
--local dbg = require("debugger")

--function that searches for the last index in the mailbox,
--that is bigger or equal with startIdx.
--if there is no id bigger, it returns startIdx.
local function getLastEmailIdx(connection, startIdx)
    local command = email.imapCommands.lastEmail
    local response = connection:executeCommand(command)
    local idx, err = email.imapParsers.lastEmail(response)
  
    if err then
        idx = startIdx
    end
    
    return idx
end

--function that gets the email at the given index.
local function fetchEmail(connection, idx)
    local output = nil
    local err = nil
    
    local header = {
        command = email.imapCommands.fetchHeader(idx),
        response = nil
    }
    local body = {
        command = email.imapCommands.fetchBody(idx),
        response = nil
    }
    
    header.response, err = connection:executeCommand(header.command)
    body.response, err = connection:executeCommand(body.command)
     
    if header.response and body.response then
        output = email.imapParsers.fetch(header.response, body.response)
    end
    
    return output, err
end

function fetchEmails(connection, startIdx)
    assert(connection)
    
    local readEmailIdx = 0
    local lastIdx = 0
    local output = {}
    local continue = true
    startIdx = startIdx or 1
    
    while continue do        
        lastIdx = getLastEmailIdx(connection, startIdx)
        assert(lastIdx, "Error executing SELECT INBOX IMAP command")
     
        for readEmailIdx = startIdx, lastIdx, 1 do
            local e = fetchEmail(connection, readEmailIdx)
            if e then
                table.insert(output, e)
            end
        end
        
        continue = coroutine.yield(output)
        
        --prepare for the next iteration
        output ={}
        if startIdx ~= lastIdx then
            startIdx = lastIdx + 1
        end
    end
    
end

local function sleep(seconds)
    local now = os.clock()
    
    while os.clock() - now < seconds do
        --nothing
    end
end

local function test()
    local user = io.read()
    local pass = io.read()
    
    local imapConnection = email.makeImap(true,
        email.servers.yahooImap,
        email.ports.imapSsl,
        user,
        pass)
    assert(imapConnection)
    
    local smtpConnection = email.makeSmtp(true,
        email.servers.yahooSmtp,
        email.ports.smtpSsl,
        "USER",
        "PASS")
    assert(smtpConnection)
    
    local emailGenerator = coroutine.create(fetchEmails)
    local try = 1
    local _, emails = coroutine.resume(emailGenerator, imapConnection)

    while try < 5 do
        print("~~~~~~~~~~~~~~~~~~~~~~~~Begin iteration~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
        for k, v in pairs(emails) do
            print(k, v.subject)
        end
        print("~~~~~~~~~~~~~~~~~~~~~~~~End iteration~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
        
        _, emails = coroutine.resume(emailGenerator, true)
        smtpConnection:sendEmail("SENDER",
            "RECEIVER",
            "Test emailFetcher take 100-" .. try,
            "It works")
        sleep(120)
        try = try + 1
    end
end

--test()
