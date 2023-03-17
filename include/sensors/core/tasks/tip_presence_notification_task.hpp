namespace sensors {
namespace tasks {

using TaskMessage = motor_control_task_messages::MoveStatusReporterTaskMessage;

template <can::message_writer_task::TaskClient CanClient, class OwnQueue>
class TipPresenceNotificationHandler {
  public:
    explicit TipPresenceNotificationHandler(
        CanClient &can_client, OwnQueue &own_queue,
        sensors::hardware::SensorHardwareBase &hardware) {}
    TipPresenceNotificationHandler(const TipPresenceNotificationHandler &) = delete;
    TipPresenceNotificationHandler(const TipPresenceNotificationHandler &&) = delete;
    auto operator=(const TipPresenceNotificationHandler &)
        -> TipPresenceNotificationHandler & = delete;
    auto operator=(const TipPresenceNotificationHandler &&)
        -> TipPresenceNotificationHandler && = delete;
    ~TipPresenceNotificationHandler() = default;

    void handle_message(const tip_presence::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(const std::monostate &) {}

    void visit(const tip_presence::TipStatusChangeDetected &m) {
        can_client.send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
    }
  private:
    CanClient& can_client;
    sensors::hardware::SensorHardwareBase hardware;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<utils::TaskMessage>, utils::TaskMessage>
class TipPresenceNotificationTask {
  public:
    using Messages = utils::TaskMessage;
    using QueueType = QueueImpl<utils::TaskMessage>;
    TipPresenceNotificationTask(can::ids::SensorId id)
        : sensor_id{id} {}
    TipPresenceNotificationTask(const TipPresenceNotificationTask &c) = delete;
    TipPresenceNotificationTask(const TipPresenceNotificationTask &&c) = delete;
    auto operator=(const TipPresenceNotificationTask &c) = delete;
    auto operator=(const TipPresenceNotificationTask &&c) = delete;
    ~TipPresenceNotificationTask() = default;

    /**
     * Task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(CanClient *can_client,
        sensors::hardware::SensorHardwareBase *hardware) {
        auto handler = TipPresenceNotificationHandler{
            *can_client, get_queue(), *hardware};
        utils::TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    void set_queue(QueueType* q) { queue = q; }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    can::ids::SensorId sensor_id;
    QueueType* queue{nullptr};
};

	} // namespace tasks
} // namespace sensors
