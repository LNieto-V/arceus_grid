FROM eclipse-mosquitto:2.0.18

# Copy custom configuration
COPY config/mosquitto.conf /mosquitto/config/mosquitto.conf

# Create data and log directories
RUN mkdir -p /mosquitto/data /mosquitto/log && \
    chown -R mosquitto:mosquitto /mosquitto

EXPOSE 1883 9001

CMD ["/usr/sbin/mosquitto", "-c", "/mosquitto/config/mosquitto.conf"]
