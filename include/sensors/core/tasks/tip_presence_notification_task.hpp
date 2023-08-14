namespace sensors {
namespace tasks {

template <can::message_writer_task::TaskClient CanClient>
class TipPresenceNotificationHandler {
  public:
    explicit TipPresenceNotificationHandler(
        CanClient &can_client, sensors::hardware::SensorHardwareBase &hardware,
        const can::ids::SensorId &id)
        : can_client{can_client}, hardware{hardware}, sensor_id{id} {}
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
                    static_cast<uint8_t>(hardware.check_tip_presence()),
                .sensor_id = sensor_id,
            });
    }

    void visit(const can::messages::TipStatusQueryRequest &m) {
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::PushTipPresenceNotification{
                .message_index = m.message_index,
                .ejector_flag_status =
                    static_cast<uint8_t>(hardware.check_tip_presence()),
                .sensor_id = sensor_id});
    }

  private:
    CanClient &can_client;
    sensors::hardware::SensorHardwareBase &hardware;
    can::ids::SensorId sensor_id;
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
    TipPresenceNotificationTask(QueueType &queue, can::ids::SensorId id)
        : queue{queue}, sensor_id{id} {}
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
        auto handler =
            TipPresenceNotificationHandler{*can_client, *hardware, sensor_id};
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
    can::ids::SensorId sensor_id;
};

}  // namespace tasks
}  // namespace sensors
