/*
 *  (c) Copyright 2016 Hewlett Packard Enterprise Development LP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License. You may obtain
 *  a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

/**********************************
*System Includes
**********************************/
#include <string>
#include <sys/un.h>
#include <string.h>
#include <sys/socket.h>
#include <gtest/gtest.h>

/**********************************
*Local Includes
**********************************/
#include "ops-ipsecd.h"
#include "ErrorNotifySS.h"
#include "mocks/mock_ISystemCalls.h"

/**********************************
*Using
**********************************/
using ::testing::_;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;
using ::testing::StrEq;
using ::testing::IsNull;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::NotNull;
using ::testing::InSequence;
using ::testing::SetArgPointee;

class FakeListener : public IErrorListener
{
    public:

        ipsec_error m_err;

        void error_event(const ipsec_error& err) override
        {
            m_err = err;
        }
};

class ErrorNotifySS_EnO : public ErrorNotifySS
{
    public:
        ErrorNotifySS_EnO(ISystemCalls& system_calls, IErrorListener& error_listener)
            : ErrorNotifySS(system_calls, error_listener)
        {
        }

        void call_cleanup()
        {
            cleanup();
        }

        void set_is_ready(bool value)
        {
            m_is_ready = value;
        }

        void set_is_running(bool value)
        {
            m_is_running = value;
        }

        void set_conn(int value)
        {
            m_conn = value;
        }

        int get_conn() const
        {
            return m_conn;
        }

        std::thread& get_error_thread()
        {
            return m_error_thread;
        }

        ipsec_ret call_run_error_receiver()
        {
            return run_error_receiver();
        }

        void call_process_error(const error_notify_msg_t& msg)
        {
            process_error(msg);
        }
};

class FakeCalls
{
    public:

        ErrorNotifySS_EnO* m_error_notify = nullptr;
        error_notify_msg_t m_msg = { 0 };

        ssize_t s_read_end(int fd, void *buf, size_t count)
        {
            if(m_error_notify != nullptr)
            {
                m_error_notify->set_is_running(false);
            }

            return sizeof(error_notify_msg_t);
        }

        int s_connect_init(int sockfd, const struct sockaddr *addr,
                           socklen_t addrlen)
        {
            if(addr == nullptr)
            {
                return -1;
            }

            const struct sockaddr_un* unix_socket = (const struct sockaddr_un*)addr;

            EXPECT_EQ(unix_socket->sun_family, AF_UNIX);
            EXPECT_EQ(strcmp(unix_socket->sun_path, ERROR_NOTIFY_SOCKET),  0);

            return 1;
        }

        ssize_t s_read_receive_err_set_msg(int fd, void *buf, size_t count)
        {
            memcpy(buf, &m_msg, sizeof(error_notify_msg_t));

            return sizeof(error_notify_msg_t) / 2;
        }
};

class ErrorNotifySSTestSuite : public Test
{
    public:

        MockISystemCalls m_system_calls;
        ErrorNotifySS_EnO m_err_notify;
        FakeListener m_fake_listener;

        ErrorNotifySSTestSuite()
            : m_err_notify(m_system_calls, m_fake_listener)
        {
        }

        void SetUp() override
        {
            m_err_notify.set_is_ready(false);
            m_err_notify.set_is_running(false);
            m_err_notify.set_conn(0);
        }

        void TearDown() override
        {
            m_err_notify.set_is_ready(false);
            m_err_notify.set_is_running(false);
            m_err_notify.set_conn(0);
        }
};

/**
 * Objective: Verify that clean up will stop the thread and close the socket
 **/
TEST_F(ErrorNotifySSTestSuite, TestCleanUp)
{
    int conn = 100;

    /////////////////////////////////////////////////

    EXPECT_FALSE(m_err_notify.get_is_ready());
    EXPECT_FALSE(m_err_notify.get_is_running());
    EXPECT_EQ(m_err_notify.get_conn(), 0);

    m_err_notify.set_is_ready(true);
    m_err_notify.set_is_running(true);
    m_err_notify.set_conn(conn);

    EXPECT_TRUE(m_err_notify.get_is_ready());
    EXPECT_TRUE(m_err_notify.get_is_running());
    EXPECT_EQ(m_err_notify.get_conn(), conn);

    /////////////////////////////////////////////////

    EXPECT_CALL(m_system_calls, s_close(Eq(conn)))
            .WillOnce(Return(0));

    /////////////////////////////////////////////////

    m_err_notify.call_cleanup();

    /////////////////////////////////////////////////

    EXPECT_FALSE(m_err_notify.get_is_ready());
    EXPECT_FALSE(m_err_notify.get_is_running());
    EXPECT_EQ(m_err_notify.get_conn(), 0);
}