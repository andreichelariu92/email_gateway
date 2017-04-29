email = require("lib_lua_email")

email.ports = {
    smtpSsl = 465,
    smtpNoSsl = 25,
    imapSsl = 993,
    imapNoSsl = 143
}

email.servers = {
    yahooImap = "imaps://imap.mail.yahoo.com/INBOX",
    yahooSmtp = "smtps://smtp.mail.yahoo.com",
    gmailImap = "imaps://imap.gmail.com/INBOX",
    gmailSmtp = "smtps://smtp.gmail.com"
}

email.imapCommands = {
    searchLast= "SELECT INBOX",
    fetchHeader = function (id) return string.format("FETCH %d (BODY[HEADER.FIELDS (TO FROM SUBJECT)])", id) end,
    fetchBody = function (id) return string.format("FETCH %d BODY[TEXT]", id) end,
    searchNew = "SEARCH NEW"
}

email.imapParsers = {}

function email.imapParsers.searchLast(response)    
    local pattern = "%* (%d+) EXISTS"
    local output = nil
    local err = nil
    
    if response then
        output = string.match(response, pattern)
        output = output and (tonumber(output) or nil)
    end
    if not output then
        err = "searchLast command has failed"
    end
    
    return output, err
end

function email.imapParsers.fetch(header, body)    
    local output = nil
    local error = "INVALID header and body"
    
    local from = nil
    local to = nil
    local subject = nil
    local content = nil
    local contentStart = nil
    
    local fromPattern = "From: <(.-)>"
    local toPattern = "To: <(.-)>"
    local subjectPattern = "Subject: (.-)\r\n"
    
    if header then
        from = string.match(header, fromPattern)
        to = string.match(header, toPattern)
        subject = string.match(header, subjectPattern)
    end
    
    if body then
        contentStart = string.find(body, "}")
        contentStart = contentStart and ((contentStart + 1) or nil)
        content = contentStart and (string.sub(body, contentStart) or nil)
    end
    
    if from and to and subject and content then
        output = {}
        output.from = from
        output.to = to
        output.subject = subject
        output.content = content
        
        error = nil
    end
    
    return output, error
end

function email.imapParsers.searchNew(response)    
    local id = nil
    local error = "There is no new email in mailbox."
    local pattern = "SEARCH (%d+)"
    
    if response then
        id = string.match(response, pattern)
        id = id and tonumber(id) or nil
    end
    
    if id then
        error = nil
    end
   
    return id, error
end

local function runTests()
    --test the default ports
    assert(email.ports.smtpSsl == 465)
    assert(email.ports.smtpNoSsl == 25)
    assert(email.ports.imapSsl == 993)
    assert(email.ports.imapNoSsl == 143)
    
    -- test usual email servers (yahoo, gmail)
    assert(email.servers.yahooImap == "imaps://imap.mail.yahoo.com/INBOX")
    assert(email.servers.yahooSmtp == "smtps://smtp.mail.yahoo.com")
    assert(email.servers.gmailImap == "imaps://imap.gmail.com/INBOX")
    assert(email.servers.gmailSmtp == "smtps://smtp.gmail.com")
    
    --test simple imap commands
    assert(email.imapCommands.searchLast == "SELECT INBOX")
    assert(email.imapCommands.searchNew == "SEARCH NEW")
    
    local user = io.read()
    local pass = io.read()
    assert(user and pass)
    local connection = email.makeImap(true,
        email.servers.yahooImap,
        email.ports.imapSsl,
        user,
        pass)
    assert(connection)
    
    --test searchLast command
    local command = email.imapCommands.searchLast
    local response = connection:executeCommand(command)
    local id, err = email.imapParsers.searchLast(response)
    assert(id and not err)
    
    --test fetch command
    command = email.imapCommands.fetchHeader(3)
    local header = connection:executeCommand(command)
    command = email.imapCommands.fetchBody(3)
    local body = connection:executeCommand(command)
    local e, err2 = email.imapParsers.fetch(header, body)
    assert(e.to and e.from and e.subject and e.content)
    
    --test search command
    command = email.imapCommands.searchNew
    response, err2 = connection:executeCommand(command)
    local success, err3 = email.imapParsers.searchNew(response)
    assert(success or err3)
        
    print("All tests went OK")
end

--runTests()

return email