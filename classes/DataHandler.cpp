//
//  DataHandler.cpp
//  Crowd
//
//  Created by Victor Norgren on 8/9/13.
//
//

#include "DataHandler.hpp"

DataHandler::DataHandler() {
    // Initialize Asio Transport
    m_server.init_asio();
    
    // Disable logging for now
    m_server.clear_access_channels(websocketpp::log::alevel::all);
    
    // Register handler callbacks
    m_server.set_open_handler(bind(&DataHandler::on_open,this,::_1));
    m_server.set_close_handler(bind(&DataHandler::on_close,this,::_1));
    m_server.set_message_handler(bind(&DataHandler::on_message,this,::_1,::_2));
};

void DataHandler::run() {
    // listen on specified port
    //uint16_t port
    m_server.listen(9002);
    
    // Start the server accept loop
    m_server.start_accept();
    
    // Start the ASIO io_service run loop
    try {
        m_server.run();
    } catch (const std::exception & e) {
        std::cout << e.what() << std::endl;
    } catch (websocketpp::lib::error_code e) {
        std::cout << e.message() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}

void DataHandler::on_open(connection_hdl hdl) {
    unique_lock<mutex> lock(m_action_lock);
    //std::cout << "on_open" << std::endl;
    m_actions.push(action(SUBSCRIBE,hdl));
    lock.unlock();
    m_action_cond.notify_one();
}

void DataHandler::on_close(connection_hdl hdl) {
    unique_lock<mutex> lock(m_action_lock);
    std::cout << "on_close" << std::endl;
    m_actions.push(action(UNSUBSCRIBE,hdl));
    lock.unlock();
    m_action_cond.notify_one();
}

void DataHandler::on_message(connection_hdl hdl, server::message_ptr msg) {
    // queue message up for sending by processing thread
    unique_lock<mutex> lock(m_action_lock);
    //std::cout << "on_message" << std::endl;
    std::cout << msg->get_payload() << std::endl;
    m_actions.push(action(MESSAGE,msg));
    lock.unlock();
    m_action_cond.notify_one();
}

void DataHandler::send_message(std::string str) {
    unique_lock<mutex> lock(m_action_lock);
    //std::cout << "on_open" << std::endl;

    con_list::iterator it;
    for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        m_server.send(*it, str, websocketpp::frame::opcode::TEXT);
    }
    lock.unlock();
    m_action_cond.notify_one();
}

void DataHandler::process_messages() {
    while(1) {
        unique_lock<mutex> lock(m_action_lock);
        
        while(m_actions.empty()) {
            m_action_cond.wait(lock);
        }
        
        action a = m_actions.front();
        m_actions.pop();
        
        lock.unlock();
        
        if (a.type == SUBSCRIBE) {
            unique_lock<mutex> lock(m_connection_lock);
            m_connections.insert(a.hdl);
        } else if (a.type == UNSUBSCRIBE) {
            unique_lock<mutex> lock(m_connection_lock);
            m_connections.erase(a.hdl);
        } else if (a.type == MESSAGE) {
            unique_lock<mutex> lock(m_connection_lock);
            
            con_list::iterator it;
            for (it = m_connections.begin(); it != m_connections.end(); ++it) {
                m_server.send(*it,a.msg);
            }
        } else {
            // undefined.
        }
    }
}