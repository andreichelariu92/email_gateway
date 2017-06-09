--local dbg = require("debugger")
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
    fetchHeader = function (id) return string.format("FETCH %d (BODY[HEADER.FIELDS (TO FROM SUBJECT)])", id) end,
    fetchBody = function (id) return string.format("FETCH %d BODY[TEXT]", id) end,
    search = function (id) return string.format("SEARCH %d:*", id) end,
    lastEmail = "SELECT INBOX"
}

email.imapParsers = {}

function email.imapParsers.fetch(header, body)
    
    local output = nil
    local error = "INVALID header and body"
    
    local from = nil
    local to = nil
    local subject = nil
    local content = nil
    local contentStart = nil
    
    local fromPattern = "From:.-<(.-)>"
    local toPattern = "To:.-<(.-)>"
    local subjectPattern = "Subject: (.-)\r\n"
    
    if header then
        from = string.match(header, fromPattern)
        to = string.match(header, toPattern)
        subject = string.match(header, subjectPattern)
    end
    
    if body then
        contentStart = string.find(body, "}")
        contentStart = contentStart and (contentStart + 1) or nil
        content = contentStart and string.sub(body, contentStart) or nil
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

function email.imapParsers.search(response)        
    local error = "There are no new emails in mailbox."
    local list = {}
    local pattern = "(%d+)"
    for id in string.gmatch(response, pattern) do
        if tonumber(id) then
            table.insert(list, tonumber(id))
            error = nil
        end
    end
    
    return list, error
end

function email.imapParsers.lastEmail(response)
    local pattern = "* (%d+) EXISTS"
    local output = nil
    local error = "INVALID RESPONSE"
    
    output = string.match(response, pattern)
    if output then
        error = nil
    end
    
    return output, error
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
        
    local user = io.read()
    local pass = io.read()
    assert(user and pass)
    local connection = email.makeImap(true,
        email.servers.yahooImap,
        email.ports.imapSsl,
        user,
        pass)
    assert(connection)
        
    --test fetch command
    command = email.imapCommands.fetchHeader(8)
    local header, err = connection:executeCommand(command)
    command = email.imapCommands.fetchBody(8)
    local body = connection:executeCommand(command)
    local e, err2 = email.imapParsers.fetch(header, body)
    assert(e.to and e.from and e.subject and e.content)
    
    --test search command
    command = email.imapCommands.search(3)    
    response, err2 = connection:executeCommand(command)
    local list, err3 = email.imapParsers.search(response)
    assert(#list or err3)
    
    --test lastEmail command
    command = email.imapCommands.lastEmail
    response, err = connection:executeCommand(command)
    local idx = email.imapParsers.lastEmail(response)
    assert(tonumber(idx))
        
    print("All tests went OK")
end

--runTests()

return email
