#
# Outputs
#
# Author: Marc Plouhinec
#

output "ctracker_ecs_public_ip" {
  value = "${alicloud_instance.ctracker_ecs.public_ip}"
}