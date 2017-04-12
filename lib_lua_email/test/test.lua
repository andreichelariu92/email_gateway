local email = require("lib_lua_email")
local noError = true

--test case 1
--create SSL smtp connection with invalid number
--of parameters
--the code should throw error
noError = pcall(function ()
    return email.makeSmtp(true,
        "localhost",
        8080)
    end
)
assert(noError == false)

--test case 2
-- create NO_SSL smtp connection with invalid number
-- of parameters
-- the code should throw an error
noError = pcall(function ()
    return email.makeSmtp(false,
        "localhost")
    end
)
assert(noError == false)

function getIp(index)
    if index == 1 then
        return nil
    else
        return "127.0.0.1"
    end
end

function getPort(index)
    if index == 2 then
        return nil
    else
        return 563
    end
end

function getUser(index)
    if index == 3 then
        return nil
    else
        return "user"
    end
end

function getPass(index)
    if index == 4 then
        return nil
    else
        return "pass"
    end
end

--test case 3
-- create a SSL connection with an invalid parameter
-- the function should throw an error
for index = 1, 4, 1 do
    noError = pcall(function ()
        return email.makeSmtp(true,
            getIp(index),
            getPort(index),
            getUser(index),
            getPass(index))
        end
    )
    assert(noError == false)
end

--test case 4
-- create NO_SSL connection with an invalid parameter
-- the function should throw an error
for index = 1, 2, 1 do
    noError = pcall(function ()
        return email.makeSmtp(false,
            getIp(index),
            getPort(index))
        end
    )
    assert(noError == false)
end

--test case 5
-- create SSL connection with valid parameters
-- the return value should be different from nil
local validSslConnection = email.makeSmtp(true,
    "127.0.0.1",
    563,
    "user",
    "pass")
assert(validSslConnection ~= nil)

--test case 6
-- create a NO_SSL connection with valid parameters
-- the return value should be different from nil
local validNoSslConnection = email.makeSmtp(false,
    "127.0.0.1",
    563)
assert(validNoSslConnection ~= nil)

local to = {nil, "test@test.com", "test@test.com", "test@test.com"}
local from = {"test@test.com", nil, "test@test.com", "test@test.com"}
local subject = {"Something", "Something", nil, "Something"}
local content = {"content", "content", "content", nil}

--test case 7
-- send an email with an invalid field
-- the function should throw error
for index = 1, 4, 1 do
    noError = pcall(function ()
        return validSslConnection:sendEmail(to[index],
            from[index],
            subject[index],
            content[index])
        end
    )
    assert(noError == false)
end

--test case 8
-- send an email with valid fields on the local host
-- the function should return false, because there
-- is no SMTP server on local host.
noError = validSslConnection:sendEmail("to@test.com",
    "from@test.com",
    "subject",
    "content")
assert(noError == false)

--test case 9
-- create an imap connection with invalid parameters (NON_SSL)
-- the function should throw error
noError = pcall(function()
    return email.makeImap(false,
        "127.0.0.1")
    end
)
assert(noError == false)

--test case 10
-- create an imap connection (SSL) with invalid parameters
-- the function should throw error.
noError = pcall(function()
    return email.makeImap(true,
        "127.0.0.1",
        456,
        "user")
    end
)
assert(noError == false)

--test case 11
-- create a valid imap connection (SSL)
-- perform a command with invalid parameters
-- the function should throw error.
local imapConnection = email.makeImap(true,
    "127.0.0.1",
    993,
    "user",
    "pass")
noError = pcall(function() return imapConnection:executeCommand() end)
assert(noError == false)

--test case 12
-- perform a valid command on an imap connection.
-- the return value should be an empty string because there is no
-- imap server on local machine.
local response, error = imapConnection:executeCommand("SELECT INBOX")
assert(response == nil and error ~= nil)

print("All tests went OK :)")