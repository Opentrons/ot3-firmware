namespace sensors {
namespace tasks {

template <can::message_writer_task::TaskClient CanClient>
class TipPresenceNotificationHandler {
  public:
    explicit TipPresenceNotificationHandler(
        CanClient &can_client, sensors::hardware::SensorHardwareBase &hardware)
        : can_client{can_client}, hardware{hardware} {}
    TipPresenceNotificationHandler(const TipPresenceNotificationHandler &) =
        delete;
    TipPresenceNotificationHandler(const TipPresenceNotificationHandler &&) =
        delete;
    auto operator=(const TipPresenceNotificationHandler &)
        -> TipPresenceNotificationHandler & = delete;
    auto operator=(const TipPresenceNotificationHandler &&)
        -> TipPresenceNotificationHandler && = delete;
    ~TipPresenceNotificationHandler() = default;

    void handle_message(const tip_presence::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(const std::monostate &) {}

    void visit(const tip_presence::TipStatusChangeDetected &) {
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::PushTipPresenceNotification{
                .message_index = 0,
                .ejector_flag_status =
                    static_cast<uint8_t>(hardware.check_tip_presence())});
    }

    void visit(const can::messages::TipStatusQueryRequest &) {
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::PushTipPresenceNotification{
                .message_index = 0,
                .ejector_flag_status =
                    static_cast<uint8_t>(hardware.check_tip_presence())});
    }

  private:
    CanClient &can_client;
    sensors::hardware::SensorHardwareBase &hardware;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<tip_presence::TaskMessage>,
                      tip_presence::TaskMessage>
class TipPresenceNotificationTask {
  public:
    using Messages = tip_presence::TaskMessage;
    using QueueType = QueueImpl<Messages>;
    TipPresenceNotificationTask(QueueType &queue) : queue{queue} {}
    TipPresenceNotificationTask(const TipPresenceNotificationTask &c) = delete;
    TipPresenceNotificationTask(const TipPresenceNotificationTask &&c) = delete;
    auto operator=(const TipPresenceNotificationTask &c) = delete;
    auto operator=(const TipPresenceNotificationTask &&c) = delete;
    ~TipPresenceNotificationTask() = default;

    /**
     * Task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(
        CanClient *can_client,
        sensors::hardware::SensorHardwareBase *hardware) {
        auto handler = TipPresenceNotificationHandler{*can_client, *hardware};
        Messages message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};

}  // namespace tasks
}  // namespace sensors
