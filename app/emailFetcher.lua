require("email")

local function getLastEmailIdx(connection)
    local command = email.imapCommands.searchLast
    local response, err = connection:executeCommand(command)
    local id = nil
    
    if response then
        id, err = email.imapParsers.searchLast(response)
    end
    
    return id, err
end

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
    local inboxEmailIdx = 0
    local output = {}
    local continue = true
    startIdx = startIdx or 1
    
    while continue do
        inboxEmailIdx = getLastEmailIdx(connection)
        assert(inboxEmailIdx, "Error executing SELECT INBOX IMAP command")
     
        for readEmailIdx = startIdx, inboxEmailIdx, 1 do
            local e = fetchEmail(connection, readEmailIdx)
            if e then
                table.insert(output, e)
            end
        end
        
        continue = coroutine.yield(output)
        
        --prepare for the next iteration
        output ={}
        startIdx = inboxEmailIdx + 1
    end
    
end

local function test()
    local user = io.read()
    local pass = io.read()
    local connection = email.makeImap(true,
        email.servers.yahooImap,
        email.ports.imapSsl,
        user,
        pass)
    
    local emailGenerator = coroutine.create(fetchEmails)
    local try = 1
    local _, emails = coroutine.resume(emailGenerator, connection)
    while try < 3 do
        for k, v in pairs(emails) do
            print(k, v.subject)
        end
    
        _, emails = coroutine.resume(emailGenerator, true)
        try = try + 1
    end
end

test()