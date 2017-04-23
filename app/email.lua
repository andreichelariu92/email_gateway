email = require("lib_lua_email")

email.ports = {
    smtpSsl = 465,
    smtpNoSsl = 25,
    imapSsl = 993,
    imapNoSsl = 143
}

email.servers = {
    yahooImap = "imaps://imap.mail.yahoo.com",
    yahooSmtp = "smtps://smtp.mail.yahoo.com",
    gmailImap = "imaps://imap.gmail.com",
    gmailSmtp = "smtps://smtp.gmail.com"
}

email.imapCommands = {
    login = "SELECT INBOX",
    fetchHeader = function (id) return string.format("FETCH %d (BODY[HEADER.FIELDS (TO FROM SUBJECT)])", id) end,
    fetchBody = function (id) return string.format("FETCH %d BODY[TEXT]", id) end,
    logout = "LOGOUT"
}

email.imapParsers = {}

function email.imapParsers.login(response)
    local pattern = "%* (%d+) EXISTS"
    local output = string.match(response, pattern)
    local err = nil
    
    if not output then
        err = "SELECT INBOX command has failed"
    end
    
    return output, err
end

function email.imapParsers.fetch(header, body)
    local output = nil
    local error = "INVALID header and body"
    
    local fromPattern = "From: <(.-)>"
    local from = string.match(header, fromPattern)
    
    local toPattern = "To: <(.-)>"
    local to = string.match(header, toPattern)
    
    local subjectPattern = "Subject: (.-)\r\n"
    local subject = string.match(header, subjectPattern)
    
    local contentStart = string.find(body, "}")
    local content = nil
    if contentStart then
        contentStart = contentStart + 1
        content = string.sub(body, contentStart)
    end
    
    if from and to and subject and content then
        output = {}
        output.from = from
        output.to = to
        output.subject = subject
        output.content = content
        
        error = nil
    end
    
    print("EXTRALOG: ", output)
    return output, error
end

local function runTests()
    --test the default ports
    assert(email.ports.smtpSsl == 465)
    assert(email.ports.smtpNoSsl == 25)
    assert(email.ports.imapSsl == 993)
    assert(email.ports.imapNoSsl == 143)
    
    -- test usual email servers (yahoo, gmail)
    assert(email.servers.yahooImap == "imaps://imap.mail.yahoo.com")
    assert(email.servers.yahooSmtp == "smtps://smtp.mail.yahoo.com")
    assert(email.servers.gmailImap == "imaps://imap.gmail.com")
    assert(email.servers.gmailSmtp == "smtps://smtp.gmail.com")
    
    --test simple imap commands
    assert(email.imapCommands.login == "SELECT INBOX")
    assert(email.imapCommands.logout == "LOGOUT")
    --assert(email.imapCommands.fetch(3) == "FETCH 3 BODY.PEEK[HEADER.subject]")
    
    local user = io.read()
    local pass = io.read()
    assert(user and pass)
    local connection = email.makeImap(true,
        email.servers.yahooImap,
        email.ports.imapSsl,
        user,
        pass)
    assert(connection)
    
    --test parseImapResponse function for login command
    local command = email.imapCommands.login
    local response = connection:executeCommand(command)
    local id, err = email.imapParsers.login(response)
    assert(id and not err)
    
    --test fetch command
    command = email.imapCommands.fetchHeader(3)
    local header = connection:executeCommand(command)
    command = email.imapCommands.fetchBody(3)
    local body = connection:executeCommand(command)
    local e, err2 = email.imapParsers.fetch(header, body)
    assert(e.to and e.from and e.subject and e.content)
    
    --test logout command
    command = email.imapCommands.logout
    response = connection:executeCommand(command)
    local success, err3 = email.imapParsers.logout(response)
    assert(success and not err3)
    
    print("All tests went OK")
end

runTests()

return email